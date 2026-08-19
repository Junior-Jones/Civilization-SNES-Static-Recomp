#!/usr/bin/env python3
"""Inventory conservative basic blocks for the V35 compact-core prototype."""
from __future__ import annotations

import argparse
import hashlib
import json
import re
from collections import Counter, defaultdict
from pathlib import Path


CONTROL = {
    "BEQ", "BNE", "BCC", "BCS", "BVC", "BVS", "BMI", "BPL", "BRA", "BRL",
    "JMP", "JML", "JSR", "JSL", "RTS", "RTL", "RTI", "BRK", "COP", "WAI",
    "STP", "MVN", "MVP",
}
WIDTH_BOUNDARY = {"REP", "SEP", "XCE", "PLP"}
ADDRESS_RE = re.compile(r"\$([0-9A-F]{2}):([0-9A-F]{4})")
HEX_RE = re.compile(r"\$([0-9A-F]{4,6})")


def parse_address(text: str) -> tuple[int, int]:
    bank, pc = text.split(":")
    return int(bank, 16), int(pc, 16)


def render_address(address: tuple[int, int]) -> str:
    return f"{address[0]:02X}:{address[1]:04X}"


def is_mmio(instruction: str) -> bool:
    for match in HEX_RE.finditer(instruction):
        value = int(match.group(1), 16) & 0xFFFFFF
        low = value & 0xFFFF
        bank = (value >> 16) & 0xFF
        if bank in (0x00, 0x80) and (0x2100 <= low <= 0x21FF or 0x4200 <= low <= 0x43FF):
            return True
        if len(match.group(1)) == 4 and (0x2100 <= low <= 0x21FF or 0x4200 <= low <= 0x43FF):
            return True
    return False


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--analysis", type=Path, required=True)
    ap.add_argument("--ownership", type=Path, required=True)
    ap.add_argument("--generated-manifest", type=Path, required=True)
    ap.add_argument("--generated-dir", type=Path, required=True)
    ap.add_argument("--out-json", type=Path, required=True)
    ap.add_argument("--out-text", type=Path, required=True)
    args = ap.parse_args()

    analysis = json.loads(args.analysis.read_text(encoding="utf-8"))
    ownership = json.loads(args.ownership.read_text(encoding="utf-8"))
    generated = json.loads(args.generated_manifest.read_text(encoding="utf-8"))
    rows_by_address: dict[tuple[int, int], list[dict]] = defaultdict(list)
    for row in analysis.get("contexts", []):
        rows_by_address[parse_address(row["address"])].append(row)

    representative = {address: rows[0] for address, rows in rows_by_address.items()}
    variants = {
        address: {(row["e"], row["m"], row["x"], row["bytes"], row["instruction"]) for row in rows}
        for address, rows in rows_by_address.items()
    }
    leaders = set()
    procedure_entries = {
        parse_address(row["entry"]) for row in ownership.get("procedures", [])
    }
    leaders.update(procedure_entries)
    leaders.update(
        parse_address(row["address"]) for row in analysis.get("interrupt_roots", [])
    )
    if analysis.get("contexts"):
        leaders.add(parse_address(analysis["contexts"][0]["address"]))

    boundary_reason: dict[tuple[int, int], set[str]] = defaultdict(set)
    for address, row in representative.items():
        bank, pc = address
        mnemonic = row["instruction"].split()[0]
        length = len(bytes.fromhex(row["bytes"]))
        fallthrough = (bank, (pc + length) & 0xFFFF)
        if (((bank << 16) | pc) >> 10) != (((bank << 16) | fallthrough[1]) >> 10):
            boundary_reason[address].add("dispatch-shard")
            if fallthrough in rows_by_address:
                leaders.add(fallthrough)
        if mnemonic in CONTROL:
            boundary_reason[address].add("control")
            if fallthrough in rows_by_address and mnemonic in {
                "BEQ", "BNE", "BCC", "BCS", "BVC", "BVS", "BMI", "BPL", "JSR", "JSL"
            }:
                leaders.add(fallthrough)
            for match in ADDRESS_RE.finditer(row["instruction"]):
                target = (int(match.group(1), 16), int(match.group(2), 16))
                if target in rows_by_address:
                    leaders.add(target)
        if mnemonic in WIDTH_BOUNDARY:
            boundary_reason[address].add("width-state")
            if fallthrough in rows_by_address:
                leaders.add(fallthrough)
        if is_mmio(row["instruction"]):
            boundary_reason[address].add("hardware-rendezvous")
            if fallthrough in rows_by_address:
                leaders.add(fallthrough)
        if len(variants[address]) > 1:
            boundary_reason[address].add("context-decode-variant")
            leaders.add(address)
            if fallthrough in rows_by_address:
                leaders.add(fallthrough)

    # Calls in ownership are authoritative even when their textual target is an
    # absolute same-bank address without an explicit bank prefix.
    for edge in ownership.get("call_edges", []):
        for field in ("callee", "continuation"):
            value = edge.get(field)
            if value:
                address = parse_address(value.split("/")[0])
                if address in rows_by_address:
                    leaders.add(address)

    blocks = []
    assigned = set()
    by_bank = defaultdict(list)
    for address in rows_by_address:
        by_bank[address[0]].append(address)
    for bank in sorted(by_bank):
        addresses = sorted(by_bank[bank], key=lambda item: item[1])
        address_set = set(addresses)
        for start in addresses:
            if start in assigned:
                continue
            current = start
            instructions = []
            reasons = set()
            while current in address_set and current not in assigned:
                assigned.add(current)
                row = representative[current]
                instructions.append(current)
                reasons.update(boundary_reason.get(current, ()))
                length = len(bytes.fromhex(row["bytes"]))
                next_address = (bank, (current[1] + length) & 0xFFFF)
                if current in boundary_reason or next_address not in address_set or (
                    next_address in leaders and next_address != start
                ):
                    break
                current = next_address
            signature = tuple(
                (representative[address]["bytes"], representative[address]["instruction"])
                for address in instructions
            )
            entry_contexts = len({
                (row["e"], row["m"], row["x"])
                for row in rows_by_address[start]
            })
            blocks.append({
                "entry": render_address(start),
                "last_instruction": render_address(instructions[-1]),
                "instruction_count": len(instructions),
                "entry_context_count": entry_contexts,
                "end_reasons": sorted(reasons) or ["gap-or-next-leader"],
                "signature_sha256": hashlib.sha256(repr(signature).encode("utf-8")).hexdigest(),
            })

    block_lengths = Counter(row["instruction_count"] for row in blocks)
    signatures = Counter(row["signature_sha256"] for row in blocks)
    bank_stats = {}
    for bank in sorted(by_bank):
        bank_blocks = [row for row in blocks if row["entry"].startswith(f"{bank:02X}:")]
        bank_stats[f"{bank:02X}"] = {
            "unique_instruction_addresses": len(by_bank[bank]),
            "blocks": len(bank_blocks),
            "block_entry_contexts": sum(row["entry_context_count"] for row in bank_blocks),
        }

    source_files = [path for path in args.generated_dir.rglob("*") if path.is_file()]
    generated_source_bytes = sum(path.stat().st_size for path in source_files)
    current_contexts = int(analysis.get("runtime_context_count", 0))
    block_entry_contexts = sum(row["entry_context_count"] for row in blocks)
    report = {
        "format": "civilization-v35-basic-block-inventory-v1",
        "analysis_sha256": hashlib.sha256(args.analysis.read_bytes()).hexdigest(),
        "ownership_sha256": hashlib.sha256(args.ownership.read_bytes()).hexdigest(),
        "runtime_context_count": current_contexts,
        "unique_instruction_address_count": len(rows_by_address),
        "conservative_block_count": len(blocks),
        "block_entry_context_count": block_entry_contexts,
        "potential_dispatch_entries_removed": current_contexts - block_entry_contexts,
        "potential_dispatch_reduction_percent": round(
            (current_contexts - block_entry_contexts) * 100.0 / current_contexts, 4
        ),
        "one_instruction_block_count": block_lengths.get(1, 0),
        "maximum_block_instruction_count": max(block_lengths),
        "mean_block_instruction_count": round(len(rows_by_address) / len(blocks), 4),
        "unique_block_body_count": len(signatures),
        "redundant_block_body_count": sum(count - 1 for count in signatures.values()),
        "current_generated_file_count": len(source_files),
        "current_generated_source_bytes": generated_source_bytes,
        "current_generated_source_mib": round(generated_source_bytes / 1048576, 4),
        "existing_identical_context_body_groups": generated.get("compacted_identical_body_groups"),
        "existing_redundant_context_bodies": generated.get("compacted_redundant_context_bodies"),
        "boundary_counts": dict(sorted(Counter(
            reason for reasons in boundary_reason.values() for reason in reasons
        ).items())),
        "block_length_histogram": {str(key): value for key, value in sorted(block_lengths.items())},
        "bank_stats": bank_stats,
        "blocks": blocks,
        "safety_contract": [
            "End blocks at control flow, width-state changes, MMIO rendezvous, decode variants, dispatch-shard edges, or graph gaps.",
            "Retain every proved instruction context as a legal resume entry inside its compacted block.",
            "Retain the exact source PBR:PC before every instruction for diagnostics and snapshots.",
            "Use only statically enumerated block-entry contexts; no runtime decoder or learning.",
            "This inventory is a size model, not generated production authority.",
        ],
    }
    args.out_json.parent.mkdir(parents=True, exist_ok=True)
    args.out_text.parent.mkdir(parents=True, exist_ok=True)
    args.out_json.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    text = [
        "Civilization V35 Test - Conservative Basic-Block Inventory",
        "===========================================================",
        "",
        f"Runtime contexts: {current_contexts:,}",
        f"Unique instruction addresses: {len(rows_by_address):,}",
        f"Conservative blocks: {len(blocks):,}",
        f"Block-entry contexts: {block_entry_contexts:,}",
        f"Potential dispatch entries removed: {current_contexts - block_entry_contexts:,} ({report['potential_dispatch_reduction_percent']:.2f}%)",
        f"Mean instructions per block: {report['mean_block_instruction_count']:.2f}",
        f"Maximum instructions per block: {report['maximum_block_instruction_count']:,}",
        f"One-instruction blocks: {report['one_instruction_block_count']:,}",
        f"Unique block bodies: {report['unique_block_body_count']:,}",
        f"Redundant block bodies: {report['redundant_block_body_count']:,}",
        f"Current generated tree: {generated_source_bytes:,} bytes ({report['current_generated_source_mib']:.3f} MiB), {len(source_files):,} files",
        "",
        "This is a conservative upper-bound model for block dispatch compaction.",
        "It preserves exact per-instruction PC updates and all static boundaries.",
    ]
    args.out_text.write_text("\n".join(text) + "\n", encoding="utf-8")
    print(json.dumps({
        "contexts": current_contexts,
        "blocks": len(blocks),
        "block_entry_contexts": block_entry_contexts,
        "dispatch_reduction_percent": report["potential_dispatch_reduction_percent"],
    }, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
