#!/usr/bin/env python3
"""Measure lossless factoring of generated exact-context instruction bodies.

This is an architecture estimator.  It never executes ROM bytes and does not
change the production core.  Every current generated instruction is emitted by
the production emitter, tokenized, and grouped by an identical C structure
with varying literal columns factored into frozen per-context parameters.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent / "rom"))
from civilization_rom import EXPECTED_SHA, EXPECTED_SIZE, HiRom  # noqa: E402
from generate_v33_closed_core import (  # noqa: E402
    emit_case,
    load_proofs,
    make_return_proofs,
    parse_addr,
)
from w65c816 import CpuContext, decode  # noqa: E402


LITERAL_RE = re.compile(
    r'"(?:\\.|[^"\\])*"|0x[0-9A-Fa-f]+[uUlL]*|(?<![A-Za-z_])\d+[uUlL]*'
)
ADDRESS_STRING_RE = re.compile(r'^"[0-9A-Fa-f]{2}:[0-9A-Fa-f]{4}"$')


def skeleton_and_literals(text: str) -> tuple[str, tuple[str, ...]]:
    literals: list[str] = []
    pieces: list[str] = []
    cursor = 0
    for match in LITERAL_RE.finditer(text):
        pieces.append(text[cursor:match.start()])
        token = match.group(0)
        literals.append(token)
        pieces.append("@A@" if ADDRESS_STRING_RE.match(token) else
                      ("@S@" if token.startswith('"') else "@N@"))
        cursor = match.end()
    pieces.append(text[cursor:])
    return "".join(pieces), tuple(literals)


def parameter_columns(rows: list[tuple[str, ...]]) -> tuple[int, int, int]:
    if not rows:
        return 0, 0, 0
    width = len(rows[0])
    if any(len(row) != width for row in rows):
        raise RuntimeError("literal-column mismatch inside one skeleton")
    varying_vectors = []
    address_columns = 0
    string_columns = 0
    for index in range(width):
        vector = tuple(row[index] for row in rows)
        if len(set(vector)) == 1:
            continue
        if all(ADDRESS_STRING_RE.match(value) for value in vector):
            address_columns += 1
            continue
        if all(value.startswith('"') for value in vector):
            string_columns += 1
        varying_vectors.append(vector)
    # Repeated use of the same instruction literal (PC, operand, target, proof
    # id) shares one packed parameter rather than consuming another word.
    unique_vectors = []
    for vector in varying_vectors:
        if vector not in unique_vectors:
            unique_vectors.append(vector)
    return len(unique_vectors), address_columns, string_columns


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("rom", type=Path)
    parser.add_argument("--analysis", type=Path, required=True)
    parser.add_argument("--indirect-proof", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()

    raw = args.rom.read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    if len(raw) != EXPECTED_SIZE or digest != EXPECTED_SHA:
        raise SystemExit("wrong Civilization ROM")
    analysis = json.loads(args.analysis.read_text(encoding="utf-8"))
    if analysis.get("rom_sha256") != digest:
        raise SystemExit("analysis/ROM mismatch")
    rom = HiRom(raw)
    _proof_doc, proofs = load_proofs(args.indirect_proof)
    _sets, ret_ids, _pool, _meta = make_return_proofs(analysis, rom)

    groups: dict[str, list[dict]] = defaultdict(list)
    emitted_bytes = 0
    for record in analysis.get("contexts", []):
        bank, pc = parse_addr(record["address"])
        context = CpuContext(bank, pc, int(record["e"]), int(record["m"]),
                             int(record["x"]), None)
        instruction = decode(rom.fetch, context)
        lines = emit_case(context, instruction, proofs, ret_ids)
        body = "\n".join(lines[2:])
        emitted_bytes += len(body.encode("utf-8"))
        skeleton, literals = skeleton_and_literals(body)
        groups[skeleton].append({
            "address": record["address"],
            "e": int(record["e"]),
            "m": int(record["m"]),
            "x": int(record["x"]),
            "instruction": record["instruction"],
            "literals": literals,
        })

    template_rows = []
    total_parameter_words = 0
    derived_address_columns = 0
    varying_string_columns = 0
    for template_id, (skeleton, rows) in enumerate(
            sorted(groups.items(), key=lambda item: (-len(item[1]), item[0]))):
        parameter_count, address_columns, string_columns = parameter_columns(
            [row["literals"] for row in rows]
        )
        total_parameter_words += parameter_count * len(rows)
        derived_address_columns += address_columns
        varying_string_columns += string_columns
        template_rows.append({
            "template_id": template_id,
            "context_count": len(rows),
            "parameter_words_per_context": parameter_count,
            "derived_address_literal_columns": address_columns,
            "varying_string_columns": string_columns,
            "skeleton_bytes": len(skeleton.encode("utf-8")),
            "example_address": rows[0]["address"],
            "example_instruction": rows[0]["instruction"],
        })

    contexts = len(analysis.get("contexts", []))
    dispatch_bytes = contexts * 8  # exact key + packed template/record reference
    parameter_bytes = total_parameter_words * 4
    template_source_bytes = sum(row["skeleton_bytes"] for row in template_rows)
    estimated_binary_bytes = dispatch_bytes + parameter_bytes + template_source_bytes
    histogram = Counter(row["parameter_words_per_context"] for row in template_rows)
    report = {
        "format": "civilization-v35-aot-template-factoring-audit-v1",
        "result": "PASS_ARCHITECTURE_ESTIMATE",
        "rom_sha256": digest,
        "analysis_sha256": hashlib.sha256(args.analysis.read_bytes()).hexdigest(),
        "runtime_context_count": contexts,
        "unique_template_count": len(template_rows),
        "singleton_template_count": sum(row["context_count"] == 1 for row in template_rows),
        "largest_template_context_count": max(row["context_count"] for row in template_rows),
        "emitted_semantic_body_bytes": emitted_bytes,
        "total_parameter_words": total_parameter_words,
        "mean_parameter_words_per_context": round(total_parameter_words / contexts, 4),
        "maximum_parameter_words_per_context": max(
            row["parameter_words_per_context"] for row in template_rows
        ),
        "derived_address_literal_columns": derived_address_columns,
        "varying_string_columns": varying_string_columns,
        "estimated_dispatch_bytes": dispatch_bytes,
        "estimated_parameter_bytes": parameter_bytes,
        "estimated_template_source_bytes": template_source_bytes,
        "estimated_compact_authority_bytes": estimated_binary_bytes,
        "estimated_compact_authority_mib": round(estimated_binary_bytes / 1048576, 4),
        "template_parameter_histogram": {
            str(key): value for key, value in sorted(histogram.items())
        },
        "runtime_rom_decode": False,
        "runtime_learning": False,
        "runtime_emulator_fallback": False,
        "claim_boundary": (
            "Estimate for exact-context AOT records plus shared frozen semantic "
            "templates. It is not linked production authority until generated, "
            "built, and checkpoint-equivalence tested."
        ),
        "templates": template_rows,
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({
        "contexts": contexts,
        "templates": len(template_rows),
        "mean_parameter_words": report["mean_parameter_words_per_context"],
        "max_parameter_words": report["maximum_parameter_words_per_context"],
        "estimated_mib": report["estimated_compact_authority_mib"],
    }, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
