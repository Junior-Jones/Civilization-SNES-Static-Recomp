#!/usr/bin/env python3
"""Prove that every strict independent-disassembler candidate is reconciled."""
from __future__ import annotations

import argparse
import hashlib
import json
import re
from collections import Counter
from pathlib import Path


CONTROL = {"RTS", "RTL", "RTI"}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("rom", type=Path)
    ap.add_argument("--independent", type=Path, required=True)
    ap.add_argument("--classification", type=Path, required=True)
    ap.add_argument("--analysis", type=Path, required=True)
    ap.add_argument("--ownership", type=Path, required=True)
    ap.add_argument("--mesen-log", type=Path, required=True)
    ap.add_argument("--output-json", type=Path, required=True)
    ap.add_argument("--output-text", type=Path, required=True)
    args = ap.parse_args()

    raw = args.rom.read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    independent = json.loads(args.independent.read_text(encoding="utf-8"))
    classes = json.loads(args.classification.read_text(encoding="utf-8"))
    analysis = json.loads(args.analysis.read_text(encoding="utf-8"))
    ownership = json.loads(args.ownership.read_text(encoding="utf-8"))
    if classes.get("rom_sha256") != digest or ownership.get("rom_sha256") != digest:
        raise SystemExit("ROM identity mismatch")

    strict = {
        int(value, 16)
        for value in independent["external_only_classification"]["STRICTLY_UNCLASSIFIED_START_CANDIDATE"]
    }
    owner: dict[int, dict] = {}
    overlaps = []
    rows = []
    for row in classes.get("ranges", []):
        start = int(row["start"], 16)
        end = int(row["end_exclusive"], 16)
        if not (0 <= start < end <= len(raw)):
            raise SystemExit(f"Invalid classification range: {row}")
        for offset in range(start, end):
            if offset in owner:
                overlaps.append(f"0x{offset:06X}")
            owner[offset] = row
        candidates = sorted(strict & set(range(start, end)))
        rows.append({
            **row,
            "bytes": end - start,
            "candidate_starts": len(candidates),
            "first_candidate": f"0x{candidates[0]:06X}" if candidates else None,
            "last_candidate": f"0x{candidates[-1]:06X}" if candidates else None,
        })

    unresolved = sorted(strict - set(owner))
    procedures = {p["entry"]: p for p in ownership.get("procedures", [])}
    instructions = {
        row["address"]: row["instruction"].split()[0]
        for row in analysis.get("contexts", [])
    }
    dormant_checks = {}
    for entry in ("C0:2258", "C0:2217"):
        proc = procedures.get(entry)
        if not proc:
            dormant_checks[entry] = False
            continue
        runtime = proc.get("runtime_contexts", [])
        exits = [
            item for item in runtime
            if instructions.get(item.split("/")[0]) in CONTROL
        ]
        # The ownership row ends without a return opcode; exact fixed-point call
        # propagation therefore cannot reach the physical fallthrough bytes.
        dormant_checks[entry] = bool(runtime) and not exits and all(
            not edge["from"].endswith((":2232", ":2233", ":81AD"))
            for edge in proc.get("direct_edges", [])
        )

    exact_dormant_bytes = {
        "C0:222E-C0:2233": raw[0x222E:0x2234].hex() == "222100c0286b",
        "C0:81AD-C0:81AF": raw[0x81AD:0x81B0].hex() == "4c5581",
    }
    mesen = args.mesen_log.read_text(encoding="utf-8-sig", errors="replace")
    match = re.search(
        r"V35_HEADLESS_ORACLE reason=(\S+) frames=(\d+) instructions=(\d+) "
        r"unique_pc=(\d+) unique_hash=([0-9A-F]+) candidate_physical_hits=(\d+) hit_offsets=([^\r\n]*)",
        mesen,
    )
    mesen_row = None
    if match:
        mesen_row = {
            "reason": match.group(1),
            "frames": int(match.group(2)),
            "instructions": int(match.group(3)),
            "unique_pc": int(match.group(4)),
            "unique_hash": match.group(5),
            "candidate_physical_hits": int(match.group(6)),
            "hit_offsets": match.group(7),
        }

    counts = Counter(owner[offset]["class"] for offset in strict if offset in owner)
    errors = []
    if overlaps:
        errors.append(f"overlapping classification bytes: {len(overlaps)}")
    if unresolved:
        errors.append(f"unresolved strict candidates: {len(unresolved)}")
    if not all(exact_dormant_bytes.values()):
        errors.append("dormant-code exact bytes changed")
    if not all(dormant_checks.values()):
        errors.append("nonreturning dormant-code predecessor proof changed")
    if not mesen_row or mesen_row["candidate_physical_hits"] != 0:
        errors.append("bounded Mesen confirmation missing or observed candidate execution")

    report = {
        "format": "civilization-v35-independent-candidate-reconciliation-v1",
        "result": "PASS" if not errors else "FAIL",
        "rom_sha256": digest,
        "strict_candidate_count": len(strict),
        "classified_candidate_count": len(strict) - len(unresolved),
        "unresolved_candidates": [f"0x{x:06X}" for x in unresolved],
        "classification_counts": dict(sorted(counts.items())),
        "ranges": rows,
        "exact_dormant_bytes": exact_dormant_bytes,
        "nonreturning_predecessor_checks": dormant_checks,
        "bounded_mesen_confirmation": mesen_row,
        "runtime_observation_used_as_static_proof": False,
        "errors": errors,
    }
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    args.output_text.parent.mkdir(parents=True, exist_ok=True)
    args.output_json.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    lines = [
        "Civilization V35 - Independent Candidate Reconciliation",
        "========================================================",
        "",
        f"Result: {report['result']}",
        f"Strict candidates: {len(strict):,}",
        f"Classified candidates: {len(strict) - len(unresolved):,}",
        f"Unresolved candidates: {len(unresolved):,}",
        "",
    ]
    for name, count in sorted(counts.items()):
        lines.append(f"{name}: {count:,}")
    lines += [
        "",
        "The two dormant instruction sequences are exact code bytes following",
        "source-proved nonreturning calls. They are classified but are not promoted",
        "into the reachable generated core. All remaining candidates are data/assets.",
        "The bounded Mesen result is confirmation only, never static authority.",
    ]
    args.output_text.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(json.dumps({
        "result": report["result"],
        "strict": len(strict),
        "classified": len(strict) - len(unresolved),
        "unresolved": len(unresolved),
    }, sort_keys=True))
    return 0 if not errors else 1


if __name__ == "__main__":
    raise SystemExit(main())
