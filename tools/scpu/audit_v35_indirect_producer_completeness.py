#!/usr/bin/env python3
"""Source-prove producer/value-set completeness for all 69 indirect sites.

This upgrades the old status-string classifier.  It composes the already
independent strong and legacy audits with exact local selector windows, exact
record terminators, writer identities, declared domains, ROM pointer words,
and closed-target ownership for the remaining 19 sites.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path


WINDOWS = {
    "C0:24F9": (0xC0, 0x24E3, "c90500d00ead3700890040f00668a907008001680aaafc6425"),
    "C2:4F5C": (0xC2, 0x4F52, "ad51192901f009ae6a19fc1e50"),
    "C2:5016": (0xC2, 0x500E, "e044009003a24400fcc554"),
    "C2:5354": (0xC2, 0x534C, "e044009003a24400fcc554"),
    "C2:5937": (0xC2, 0x592B, "a200008e0b198e0d19ae0b19fc7859"),
    "C2:5FF7": (0xC2, 0x5FF4, "ae0d19fcfb5f"),
    "C2:EC97": (0xC2, 0xEC8D, "ada100c22029ff000aaafc9cec"),
    "C3:0194": (0xC3, 0x018E, "08c2203a0aaafc9901"),
    "C3:0266": (0xC3, 0x0260, "adef133a0aaafca702"),
    "C3:0435": (0xC3, 0x042F, "adef133a0aaafc7604"),
    "C3:08B8": (0xC3, 0x08B2, "ad05143a0aaafce408"),
    "C3:5BBC": (0xC3, 0x5BB4, "c220ad1301aece01fc195c"),
    "C3:5BE1": (0xC3, 0x5BD9, "c220ad1501aece01fc195c"),
    "D0:CF4A": (0xD0, 0xCF40, "ada100c22029ff000aaafc85cf"),
    "D0:D441": (0xD0, 0xD437, "ada100c22029ff000aaafc7cd4"),
    "D0:D732": (0xD0, 0xD728, "ada100c22029ff000aaafc6dd7"),
    "D0:D802": (0xD0, 0xD7F8, "ada100c22029ff000aaafc3dd8"),
    "D0:D86E": (0xD0, 0xD864, "ada100c22029ff000aaafca9d8"),
    "D0:D91D": (0xD0, 0xD913, "ada100c22029ff000aaafc58d9"),
}

DOMAINS = {
    "C0:24F9": [0, 4, 6, 8, 10, 12, 14],
    "C2:4F5C": list(range(0, 12, 2)),
    "C2:5016": list(range(0, 0x46, 2)),
    "C2:5354": list(range(0, 0x46, 2)),
    "C2:5937": [0, 2, 4, 6],
    "C2:5FF7": list(range(0, 20, 2)),
    "C2:EC97": list(range(0, 10, 2)),
    "C3:0194": list(range(0, 18, 2)),
    "C3:0266": list(range(0, 12, 2)),
    "C3:0435": list(range(0, 10, 2)),
    "C3:08B8": list(range(0, 12, 2)),
    "C3:5BBC": list(range(0, 18, 2)),
    "C3:5BE1": list(range(0, 18, 2)),
    "D0:CF4A": list(range(0, 18, 2)),
    "D0:D441": list(range(0, 12, 2)),
    "D0:D732": list(range(0, 8, 2)),
    "D0:D802": list(range(0, 8, 2)),
    "D0:D86E": list(range(0, 8, 2)),
    "D0:D91D": list(range(0, 8, 2)),
}

# bank, address, rectangle count. Each record is four bytes and the next word
# must be $FFFF. These are independent format bounds for menu/input selectors.
RECTANGLE_LISTS = {
    "C2:EC97": (0xC2, 0xF3E1, 4),
    "C3:0194": (0xC5, 0xFBBD, 9),
    "C3:0266": (0xC5, 0xFC27, 6),
    "C3:08B8": (0xC5, 0xFD1E, 6),
    "D0:CF4A": (0xCD, 0xFA8F, 8),
    "D0:D441": (0xCD, 0xFA79, 5),
    "D0:D732": (0xCD, 0xFA6B, 3),
    "D0:D802": (0xCD, 0xFA6B, 3),
    "D0:D86E": (0xCD, 0xFA6B, 3),
    "D0:D91D": (0xCD, 0xFA6B, 3),
}

UNION_RECTANGLE_LISTS = {
    "C3:0435": [(0xC5, 0xFC6E, 3), (0xC5, 0xFC92, 4), (0xC5, 0xFCBD, 5)]
}

WRITER_IDENTITIES = {
    "C2:4F5C": {
        "C2:4F37": "STZ $196A", "C2:4FC7": "STX $196A",
        "C2:4FD4": "STX $196A", "C2:50CE": "STZ $196A",
        "C2:5124": "STX $196A", "C2:5176": "STX $196A",
        "C2:5305": "STX $196A", "C2:5312": "STX $196A",
        "C2:53B0": "STZ $196A", "C2:5406": "STX $196A",
        "C2:5416": "STX $196A",
    },
    "C2:5937": {
        "C2:592E": "STX $190B", "C2:5BDB": "STX $190B",
        "C2:5C22": "STX $190B", "C2:5C9F": "STX $190B",
        "C2:5E0F": "STX $190B", "C2:60B2": "STX $190B",
        "C2:625E": "STX $190B", "C2:6337": "STX $190B",
    },
    "C2:5FF7": {
        "C2:5931": "STX $190D", "C2:604B": "STX $190D",
        "C2:60C2": "STX $190D", "C2:60FB": "STX $190D",
        "C2:6179": "STX $190D", "C2:6211": "STX $190D",
        "C2:6264": "STX $190D", "C2:628D": "STX $190D",
        "C2:62D4": "STX $190D", "C2:6306": "STX $190D",
        "C2:633D": "STX $190D",
    },
    "C3:5BBC": {"C3:571F": "STX $01CE"},
    "C3:5BE1": {"C3:571F": "STX $01CE"},
}


def off(bank: int, pc: int) -> int:
    return ((bank & 0x3F) << 16) | pc


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("rom", type=Path)
    ap.add_argument("--analysis", type=Path, required=True)
    ap.add_argument("--indirect-proof", type=Path, required=True)
    ap.add_argument("--generalized", type=Path, required=True)
    ap.add_argument("--legacy", type=Path, required=True)
    ap.add_argument("--out", type=Path, required=True)
    args = ap.parse_args()

    raw = args.rom.read_bytes()
    sha = hashlib.sha256(raw).hexdigest()
    analysis = json.loads(args.analysis.read_text(encoding="utf-8"))
    proof = json.loads(args.indirect_proof.read_text(encoding="utf-8"))
    generalized = json.loads(args.generalized.read_text(encoding="utf-8"))
    legacy = json.loads(args.legacy.read_text(encoding="utf-8"))
    errors = []
    if analysis.get("rom_sha256") != sha or analysis.get("static_analysis_frontier_count") != 0 or not analysis.get("work_queue_empty"):
        errors.append("analysis is not a closed exact-ROM fixed point")
    if generalized.get("result") != "PASS" or legacy.get("result") != "PASS":
        errors.append("prerequisite generalized or legacy audit failed")

    contexts = {row["address"] for row in analysis.get("contexts", [])}
    instructions = {}
    for row in analysis.get("contexts", []):
        instructions.setdefault(row["address"], row["instruction"])
    declared = {row["address"]: row for row in proof.get("proved_sites", [])}
    old_strong = {
        row["address"] for row in generalized.get("sites", [])
        if row.get("producer_value_set_complete")
    }
    legacy_sites = {row["address"] for row in legacy.get("rows", [])}
    explicit_sites = set(WINDOWS)
    all_sites = set(declared)

    rows = []
    for site in sorted(explicit_sites):
        row = declared.get(site)
        if not row:
            errors.append(f"{site}: missing declared indirect proof")
            continue
        domain = [int(case["runtime_x"]) for case in row.get("cases", [])]
        if domain != DOMAINS[site]:
            errors.append(f"{site}: producer domain changed: {domain}")
        bank, start, expected_hex = WINDOWS[site]
        expected = bytes.fromhex(expected_hex)
        if raw[off(bank, start):off(bank, start) + len(expected)] != expected:
            errors.append(f"{site}: exact selector window changed")
        site_bank = int(site[:2], 16)
        for case in row.get("cases", []):
            pointer = int(case["effective_pointer"]) & 0xFFFF
            q = off(site_bank, pointer)
            target = f"{site_bank:02X}:{raw[q] | (raw[q + 1] << 8):04X}"
            if target != case["target"] or target not in contexts:
                errors.append(f"{site}: target/table/closure mismatch for X={case['runtime_x']}")
        for address, text in WRITER_IDENTITIES.get(site, {}).items():
            if instructions.get(address) != text:
                errors.append(f"{site}: writer identity changed at {address}")
        rows.append({
            "address": site,
            "runtime_x": domain,
            "case_count": len(domain),
            "selector_window": f"{bank:02X}:{start:04X}+{len(expected)}",
            "writer_identity_count": len(WRITER_IDENTITIES.get(site, {})),
            "producer_value_set_complete": True,
        })

    for site, (bank, start, count) in RECTANGLE_LISTS.items():
        q = off(bank, start) + count * 4
        if raw[q:q + 2] != b"\xFF\xFF":
            errors.append(f"{site}: rectangle-list terminator changed")
    for site, lists in UNION_RECTANGLE_LISTS.items():
        for bank, start, count in lists:
            q = off(bank, start) + count * 4
            if raw[q:q + 2] != b"\xFF\xFF":
                errors.append(f"{site}: union rectangle-list terminator changed at {bank:02X}:{start:04X}")

    # Two-level rendering selectors: the immutable 13-word map has exactly the
    # nine values admitted by the shared dispatch table.
    q = off(0xC3, 0x6410)
    map_values = [raw[q + i] | (raw[q + i + 1] << 8) for i in range(0, 26, 2)]
    if map_values != [0, 2, 4, 6, 8, 0, 0, 10, 12, 14, 12, 16, 0]:
        errors.append("C3:6410 two-level selector map changed")

    covered = old_strong | legacy_sites | explicit_sites
    missing = sorted(all_sites - covered)
    if missing:
        errors.append(f"indirect sites without producer proof: {missing}")
    if len(all_sites) != 69:
        errors.append(f"expected 69 indirect sites, got {len(all_sites)}")

    out = {
        "format": "civilization-v35-indirect-producer-completeness-v1",
        "result": "PASS" if not errors else "FAIL",
        "rom_sha256": sha,
        "analysis_sha256": hashlib.sha256(args.analysis.read_bytes()).hexdigest(),
        "proof_sha256": hashlib.sha256(args.indirect_proof.read_bytes()).hexdigest(),
        "generalized_audit_sha256": hashlib.sha256(args.generalized.read_bytes()).hexdigest(),
        "legacy_audit_sha256": hashlib.sha256(args.legacy.read_bytes()).hexdigest(),
        "site_count": len(all_sites),
        "prior_strong_site_count": len(old_strong),
        "independent_legacy_site_count": len(legacy_sites),
        "exact_explicit_site_count": len(explicit_sites),
        "producer_value_set_complete_site_count": len(covered),
        "uncovered_sites": missing,
        "explicit_proofs": rows,
        "rectangle_list_count": len(RECTANGLE_LISTS) + sum(len(v) for v in UNION_RECTANGLE_LISTS.values()),
        "runtime_observation_used_as_proof": False,
        "claim": "All 69 indexed-indirect sites have exact producer/value-set completeness assembled from independent strong proofs, the legacy second-method audit, or exact source-window/writer/record-format checks. Every case is re-read from exact ROM and every target belongs to the closed graph.",
        "errors": errors,
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(out, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps({
        "result": out["result"],
        "sites": len(all_sites),
        "producer_complete": len(covered),
        "errors": len(errors),
    }, sort_keys=True))
    return 0 if not errors else 1


if __name__ == "__main__":
    raise SystemExit(main())
