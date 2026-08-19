#!/usr/bin/env python3
"""Build a reproducible first-pass ROM/code classification audit.

This does not declare every non-code byte to be data.  It measures the existing
closed generated authority against the exact physical ROM, checks that all
manifested dynamic targets have emitted instruction starts, and leaves the rest
explicitly UNCLASSIFIED for independent data/table/padding analysis.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import sys
from collections import Counter, defaultdict
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent / "rom"))
from civilization_rom import EXPECTED_SHA, EXPECTED_SIZE, HiRom  # noqa: E402


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def parse_address(text: str) -> tuple[int, int]:
    bank, pc = text.split(":")
    return int(bank, 16), int(pc, 16)


def target_address(value) -> str | None:
    if isinstance(value, str):
        return value.split("/")[0]
    if isinstance(value, dict):
        for key in ("to", "target", "address"):
            if key in value and isinstance(value[key], str):
                return value[key].split("/")[0]
    return None


def merge_class_ranges(classes: bytearray) -> list[dict]:
    names = {
        0: "UNCLASSIFIED",
        1: "KNOWN_CODE",
        2: "SNES_HEADER_VECTOR",
        3: "PADDING_CANDIDATE",
    }
    ranges = []
    start = 0
    current = classes[0]
    for offset in range(1, len(classes) + 1):
        value = classes[offset] if offset < len(classes) else None
        if value == current:
            continue
        ranges.append({
            "start": f"0x{start:06X}",
            "end_exclusive": f"0x{offset:06X}",
            "length": offset - start,
            "class": names[current],
        })
        start = offset
        current = value
    return ranges


def padding_candidates(raw: bytes, eligible: set[int], minimum: int = 64) -> list[dict]:
    ranges = []
    offset = 0
    while offset < len(raw):
        value = raw[offset]
        if value not in (0x00, 0xFF) or offset not in eligible:
            offset += 1
            continue
        end = offset + 1
        while end < len(raw) and raw[end] == value and end in eligible:
            end += 1
        if end - offset >= minimum:
            ranges.append({
                "start": f"0x{offset:06X}",
                "end_exclusive": f"0x{end:06X}",
                "length": end - offset,
                "fill": f"0x{value:02X}",
                "status": "candidate-only-needs-format-or-reference-confirmation",
            })
        offset = end
    return ranges


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("rom", type=Path)
    parser.add_argument("--analysis", type=Path, required=True)
    parser.add_argument("--indirect-proof", type=Path, required=True)
    parser.add_argument("--output-json", type=Path, required=True)
    parser.add_argument("--output-text", type=Path, required=True)
    args = parser.parse_args()

    raw = args.rom.read_bytes()
    rom_hash = hashlib.sha256(raw).hexdigest()
    if len(raw) != EXPECTED_SIZE or rom_hash != EXPECTED_SHA:
        raise SystemExit("The exact Civilization (USA) ROM is required.")
    analysis = json.loads(args.analysis.read_text(encoding="utf-8"))
    indirect = json.loads(args.indirect_proof.read_text(encoding="utf-8"))
    if analysis.get("rom_sha256") != rom_hash:
        raise SystemExit("Analysis manifest ROM hash does not match the ROM.")

    mapper = HiRom(raw)
    contexts_by_address: dict[str, list[dict]] = defaultdict(list)
    for row in analysis.get("contexts", []):
        contexts_by_address[row["address"]].append(row)

    instruction_starts: set[int] = set()
    instruction_bytes: set[int] = set()
    byte_owners: dict[int, set[str]] = defaultdict(set)
    address_variants = 0
    decode_variant_addresses = []
    physical_aliases: dict[int, set[str]] = defaultdict(set)
    bank_stats: dict[str, Counter] = defaultdict(Counter)

    for address, rows in sorted(contexts_by_address.items()):
        bank, pc = parse_address(address)
        offset = mapper.cpu_to_offset(bank, pc)
        if offset is None:
            raise SystemExit(f"Manifested ROM context is not ROM-mapped: {address}")
        instruction_starts.add(offset)
        physical_aliases[offset].add(address)
        bank_name = f"{bank:02X}"
        bank_stats[bank_name]["instruction_addresses"] += 1
        encodings = {row["bytes"] for row in rows}
        instructions = {row["instruction"] for row in rows}
        if len(rows) > 1:
            address_variants += len(rows) - 1
        if len(encodings) > 1 or len(instructions) > 1:
            decode_variant_addresses.append({
                "address": address,
                "encodings": sorted(encodings),
                "instructions": sorted(instructions),
            })
        for encoded in encodings:
            data = bytes.fromhex(encoded)
            for index, expected in enumerate(data):
                current = (offset + index) % len(raw)
                if raw[current] != expected:
                    raise SystemExit(
                        f"ROM byte mismatch at {address}+{index}: "
                        f"manifest={expected:02X} rom={raw[current]:02X}"
                    )
                instruction_bytes.add(current)
                byte_owners[current].add(address)

    known_addresses = set(contexts_by_address)

    missing_call_targets = []
    for edge in analysis.get("call_edges", []):
        target = target_address(edge)
        if target and target not in known_addresses:
            missing_call_targets.append(target)

    missing_return_targets = []
    return_target_count = 0
    for site, targets in analysis.get("return_site_sets", {}).items():
        for target in targets:
            return_target_count += 1
            address = target_address(target)
            if address and address not in known_addresses:
                missing_return_targets.append({"site": site, "target": address})

    missing_indirect_targets = []
    indirect_target_count = 0
    proved_sites = indirect.get("proved_sites", [])
    for site in proved_sites:
        for case in site.get("cases", []):
            indirect_target_count += 1
            address = target_address(case)
            if address and address not in known_addresses:
                missing_indirect_targets.append({
                    "site": site.get("address", ""), "target": address
                })

    header_bytes = set(range(0xFFB0, 0x10000))
    header_only = header_bytes - instruction_bytes
    unclassified = set(range(len(raw))) - instruction_bytes - header_bytes
    padding = padding_candidates(raw, unclassified)
    classes = bytearray(len(raw))
    for offset in header_only:
        classes[offset] = 2
    for row in padding:
        start = int(row["start"], 16)
        end = int(row["end_exclusive"], 16)
        classes[start:end] = b"\x03" * (end - start)
    for offset in instruction_bytes:
        classes[offset] = 1
    class_ranges = merge_class_ranges(classes)
    padding_bytes = sum(row["length"] for row in padding)
    overlap_bytes = [
        {"offset": f"0x{offset:06X}", "owners": sorted(owners)}
        for offset, owners in byte_owners.items() if len(owners) > 1
    ]
    alias_starts = [
        {"offset": f"0x{offset:06X}", "addresses": sorted(addresses)}
        for offset, addresses in physical_aliases.items() if len(addresses) > 1
    ]

    per_bank = {}
    for bank, values in sorted(bank_stats.items()):
        per_bank[bank] = dict(values)

    target_checks_pass = not (
        missing_call_targets or missing_return_targets or missing_indirect_targets
    )
    result = {
        "format": "civilization-v35-test-rom-classification-audit-v1",
        "rom_sha256": rom_hash,
        "analysis_sha256": sha256(args.analysis),
        "indirect_proof_sha256": sha256(args.indirect_proof),
        "existing_authority": {
            "runtime_context_count": analysis.get("runtime_context_count"),
            "unique_cpu_instruction_addresses": len(contexts_by_address),
            "physical_instruction_starts": len(instruction_starts),
            "additional_context_variants": address_variants,
            "context_to_unique_address_ratio": round(
                len(analysis.get("contexts", [])) / len(contexts_by_address), 6
            ),
            "static_analysis_frontier_count": analysis.get(
                "static_analysis_frontier_count"
            ),
            "work_queue_empty": analysis.get("work_queue_empty"),
        },
        "physical_rom_classification": {
            "rom_bytes": len(raw),
            "known_instruction_bytes": len(instruction_bytes),
            "known_header_vector_bytes_not_code": len(header_only),
            "unclassified_bytes": len(unclassified),
            "padding_candidate_bytes_in_unclassified": padding_bytes,
            "strictly_unclassified_after_padding_candidates": len(unclassified) - padding_bytes,
            "known_instruction_percent": round(
                len(instruction_bytes) * 100.0 / len(raw), 6
            ),
            "classification_complete": False,
            "classification_note": (
                "Non-code bytes remain UNCLASSIFIED until independent table, "
                "asset, executable-template, padding, and orphan-code analysis."
            ),
        },
        "physical_class_ranges": class_ranges,
        "padding_candidates": padding,
        "control_target_checks": {
            "manifest_call_edges": len(analysis.get("call_edges", [])),
            "manifest_return_targets": return_target_count,
            "manifest_indirect_sites": len(proved_sites),
            "manifest_indirect_cases": indirect_target_count,
            "missing_call_targets": sorted(set(missing_call_targets)),
            "missing_return_targets": missing_return_targets,
            "missing_indirect_targets": missing_indirect_targets,
            "all_manifested_targets_have_generated_instruction_starts": target_checks_pass,
        },
        "decode_and_mapping_checks": {
            "addresses_with_multiple_context_decodings": decode_variant_addresses,
            "overlapping_physical_instruction_bytes": overlap_bytes,
            "mirrored_instruction_start_aliases": alias_starts,
        },
        "per_cpu_bank": per_bank,
        "next_required_audits": [
            "independently classify all unclassified ROM ranges",
            "scan for executable-looking orphan regions using a second disassembler",
            "inventory every ROM pointer/callback/script/event dispatch table",
            "complete producer/value-set evidence for the 36 remaining finite indirect sites",
            "inventory executable-WRAM templates, writers, epochs, and re-entry roots",
            "prototype basic-block compaction only after classification completeness",
        ],
        "result": "PASS_BASELINE_TARGETS_CLASSIFICATION_INCOMPLETE"
        if target_checks_pass else "FAIL_MANIFESTED_TARGET_MISSING",
    }

    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    args.output_text.parent.mkdir(parents=True, exist_ok=True)
    args.output_json.write_text(
        json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    summary = [
        "Civilization Version 35 Test - ROM Classification Baseline",
        "===========================================================",
        "",
        f"Result: {result['result']}",
        f"Exact ROM SHA-256: {rom_hash}",
        f"Runtime contexts: {analysis.get('runtime_context_count'):,}",
        f"Unique CPU instruction addresses: {len(contexts_by_address):,}",
        f"Unique physical instruction starts: {len(instruction_starts):,}",
        f"Additional context variants: {address_variants:,}",
        f"Known physical instruction bytes: {len(instruction_bytes):,} "
        f"({len(instruction_bytes) * 100.0 / len(raw):.3f}% of ROM)",
        f"Unclassified physical ROM bytes: {len(unclassified):,}",
        f"High-confidence-shape padding candidates (still unconfirmed): {padding_bytes:,}",
        f"Strictly unclassified after padding candidates: {len(unclassified) - padding_bytes:,}",
        f"Merged physical classification ranges: {len(class_ranges):,}",
        f"Manifested call edges: {len(analysis.get('call_edges', [])):,}",
        f"Manifested return targets: {return_target_count:,}",
        f"Indirect sites/cases: {len(proved_sites):,}/{indirect_target_count:,}",
        f"Missing manifested targets: "
        f"{len(missing_call_targets) + len(missing_return_targets) + len(missing_indirect_targets):,}",
        "",
        "Interpretation",
        "--------------",
        "The existing fixed-point authority is internally closed for every target",
        "already present in its manifests. This is not yet a whole-ROM classification",
        "claim: all remaining bytes are deliberately marked UNCLASSIFIED instead of",
        "being guessed to be data. The next work is independent table/data/orphan-code",
        "classification and producer-completeness analysis, not gameplay exploration.",
        "",
        "See the JSON receipt for bank counts, aliases, overlaps, and exact next audits.",
    ]
    args.output_text.write_text("\n".join(summary) + "\n", encoding="utf-8")
    print(json.dumps({
        "result": result["result"],
        "contexts": analysis.get("runtime_context_count"),
        "unique_addresses": len(contexts_by_address),
        "instruction_bytes": len(instruction_bytes),
        "unclassified_bytes": len(unclassified),
        "missing_manifested_targets": (
            len(missing_call_targets) + len(missing_return_targets)
            + len(missing_indirect_targets)
        ),
    }, sort_keys=True))
    return 0 if target_checks_pass else 1


if __name__ == "__main__":
    raise SystemExit(main())
