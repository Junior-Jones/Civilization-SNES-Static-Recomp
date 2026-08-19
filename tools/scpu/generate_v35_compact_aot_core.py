#!/usr/bin/env python3
"""Generate the compact exact-context V35 AOT S-CPU authority.

The generator uses the production semantic emitter as its only instruction
authority.  It losslessly factors identical emitted C structures into shared
templates and stores varying literals in frozen per-context records.  Runtime
execution performs an exact PBR:PC:E:M:X lookup; it never reads or decodes an
opcode from the ROM and has no learning or emulator fallback.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
import sys
from collections import defaultdict
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent / "rom"))
from audit_v35_aot_template_factoring import (  # noqa: E402
    ADDRESS_STRING_RE,
    LITERAL_RE,
    skeleton_and_literals,
)
from civilization_rom import EXPECTED_SHA, EXPECTED_SIZE, HiRom  # noqa: E402
from generate_v33_closed_core import (  # noqa: E402
    ctx_key,
    emit_case,
    load_proofs,
    make_return_proofs,
    parse_addr,
)
from w65c816 import CpuContext, decode  # noqa: E402


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def literal_to_u32(token: str) -> int:
    value = re.sub(r"[uUlL]+$", "", token)
    parsed = int(value, 0)
    if not 0 <= parsed <= 0xFFFFFFFF:
        raise RuntimeError(f"varying literal does not fit u32: {token}")
    return parsed


def analyze_columns(rows: list[dict]) -> tuple[list[str], list[tuple[str, ...]]]:
    width = len(rows[0]["literals"])
    vectors = [tuple(row["literals"][index] for row in rows)
               for index in range(width)]
    unique_variable_vectors: list[tuple[str, ...]] = []
    replacements: list[str] = []
    for vector in vectors:
        if all(ADDRESS_STRING_RE.match(value) for value in vector):
            replacements.append("NULL")
        elif len(set(vector)) == 1:
            replacements.append(vector[0])
        else:
            if any(value.startswith('"') for value in vector):
                raise RuntimeError("varying non-address string cannot be packed")
            if vector not in unique_variable_vectors:
                unique_variable_vectors.append(vector)
            replacements.append(f"p[{unique_variable_vectors.index(vector)}]")
    return replacements, unique_variable_vectors


def substitute_literals(body: str, replacements: list[str]) -> str:
    index = 0

    def replace(_match: re.Match[str]) -> str:
        nonlocal index
        value = replacements[index]
        index += 1
        return value

    result = LITERAL_RE.sub(replace, body)
    if index != len(replacements):
        raise RuntimeError("literal substitution did not consume every column")
    return result


def canonicalize_diagnostic_addresses(body: str) -> str:
    return LITERAL_RE.sub(
        lambda match: "NULL" if ADDRESS_STRING_RE.match(match.group(0))
        else match.group(0),
        body,
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("rom", type=Path)
    parser.add_argument("--analysis", type=Path, required=True)
    parser.add_argument("--indirect-proof", type=Path, required=True)
    parser.add_argument("--out-dir", type=Path, required=True)
    args = parser.parse_args()

    raw = args.rom.read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    if len(raw) != EXPECTED_SIZE or digest != EXPECTED_SHA:
        raise SystemExit("wrong Civilization ROM")
    analysis = json.loads(args.analysis.read_text(encoding="utf-8"))
    if analysis.get("rom_sha256") != digest:
        raise SystemExit("analysis/ROM mismatch")
    if (analysis.get("static_analysis_frontier_count") != 0 or
            not analysis.get("work_queue_empty") or
            analysis.get("artificial_graph_cap") is not None):
        raise SystemExit("analysis is not a closed uncapped fixed point")
    rom = HiRom(raw)
    _proof_doc, proofs = load_proofs(args.indirect_proof)
    _sets, ret_ids, _pool, _meta = make_return_proofs(analysis, rom)

    structural: dict[str, list[dict]] = defaultdict(list)
    for record in analysis.get("contexts", []):
        bank, pc = parse_addr(record["address"])
        context = CpuContext(bank, pc, int(record["e"]), int(record["m"]),
                             int(record["x"]), None)
        instruction = decode(rom.fetch, context)
        if instruction.raw.hex() != record["bytes"] or instruction.text != record["instruction"]:
            raise RuntimeError(f"exact-ROM context mismatch {record['address']}")
        body = "\n".join(emit_case(context, instruction, proofs, ret_ids)[2:])
        skeleton, literals = skeleton_and_literals(body)
        structural[skeleton].append({
            "context": context,
            "body": body,
            "literals": literals,
        })

    ordered = sorted(structural.items(), key=lambda item: (-len(item[1]), item[0]))
    templates = []
    context_records = []
    original_semantics_hash = hashlib.sha256()
    reconstructed_semantics_hash = hashlib.sha256()
    for template_id, (_skeleton, rows) in enumerate(ordered):
        replacements, vectors = analyze_columns(rows)
        template_body = substitute_literals(rows[0]["body"], replacements)
        templates.append({
            "id": template_id,
            "body": template_body,
            "parameter_count": len(vectors),
            "context_count": len(rows),
        })
        for row_index, row in enumerate(rows):
            parameters = [literal_to_u32(vector[row_index]) for vector in vectors]
            context = row["context"]
            expected = canonicalize_diagnostic_addresses(row["body"])
            reconstructed = re.sub(
                r"p\[(\d+)\]",
                lambda match: vectors[int(match.group(1))][row_index],
                template_body,
            )
            if reconstructed != expected:
                raise RuntimeError(
                    f"factored semantic reconstruction mismatch at "
                    f"{context.pbr:02X}:{context.pc:04X}"
                )
            key_bytes = ctx_key(context).to_bytes(4, "little")
            original_semantics_hash.update(key_bytes)
            original_semantics_hash.update(expected.encode("utf-8"))
            reconstructed_semantics_hash.update(key_bytes)
            reconstructed_semantics_hash.update(reconstructed.encode("utf-8"))
            context_records.append({
                "key": ctx_key(context),
                "group": (((context.pbr << 16) | context.pc) >> 10),
                "template_id": template_id,
                "parameters": parameters,
            })

    out = args.out_dir
    shards = out / "civilization_compact_aot_shards"
    if shards.exists():
        shutil.rmtree(shards)
    shards.mkdir(parents=True)

    header = """#ifndef CIVILIZATION_COMPACT_AOT_H
#define CIVILIZATION_COMPACT_AOT_H
#include <stdint.h>
#include "civilization_static_recomp.h"
int civ_compact_aot_execute(CivRecomp *i,uint16_t template_id,const uint32_t *p);
#endif
"""
    (out / "civilization_compact_aot.h").write_text(header, encoding="utf-8", newline="\n")

    template_lines = [
        "/* AUTO-GENERATED compact exact-context AOT semantic templates. */",
        "/* No ROM opcode decode, learning, or emulator fallback. */",
        '#include "civilization_internal.h"',
        '#include "civilization_generated_core.h"',
        '#include "civilization_compact_aot.h"',
        "",
    ]
    for template in templates:
        template_lines += [
            f"static int civ_aot_template_{template['id']:03X}(CivRecomp *i,const uint32_t *p){{",
            "  (void)p;",
        ]
        template_lines.extend(template["body"].splitlines())
        template_lines += ["}", ""]
    template_lines += [
        "int civ_compact_aot_execute(CivRecomp *i,uint16_t template_id,const uint32_t *p){",
        "  switch(template_id){",
    ]
    for template in templates:
        template_lines.append(
            f"  case 0x{template['id']:03X}u:return civ_aot_template_{template['id']:03X}(i,p);"
        )
    template_lines += [
        "  default:return civ_fail_frontier(i,\"Unknown compact AOT semantic template.\",NULL);",
        "  }",
        "}",
        "",
    ]
    templates_text = "\n".join(template_lines)
    (out / "civilization_compact_aot_templates.c").write_text(
        templates_text, encoding="utf-8", newline="\n"
    )

    grouped: dict[int, list[dict]] = defaultdict(list)
    for record in context_records:
        grouped[record["group"]].append(record)
    shard_hashes = {}
    parameter_words = 0
    for group, records in sorted(grouped.items()):
        records.sort(key=lambda row: row["key"])
        parameters = []
        record_lines = []
        for record in records:
            offset = len(parameters)
            if offset > 0xFFFF:
                raise RuntimeError(f"parameter shard overflow in group {group:05X}")
            parameters.extend(record["parameters"])
            record_lines.append(
                f"  {{0x{record['key']:08X}u,0x{offset:04X}u,0x{record['template_id']:03X}u}},"
            )
        parameter_words += len(parameters)
        name = f"civilization_core_group_{group:05X}"
        lines = [
            "/* AUTO-GENERATED compact exact-context AOT shard. */",
            '#include "civilization_internal.h"',
            '#include "civilization_generated_core.h"',
            '#include "civilization_compact_aot.h"',
            "",
            "typedef struct CivCompactRecord {uint32_t key;uint16_t parameter_offset;uint16_t template_id;} CivCompactRecord;",
            f"static const CivCompactRecord records[{len(records)}]={{",
        ]
        lines.extend(record_lines)
        lines += ["};"]
        if parameters:
            lines.append(f"static const uint32_t parameters[{len(parameters)}]={{")
            for start in range(0, len(parameters), 8):
                chunk = parameters[start:start + 8]
                lines.append("  " + ",".join(f"0x{value:08X}u" for value in chunk) + ",")
            lines.append("};")
        else:
            lines.append("static const uint32_t parameters[1]={0u};")
        lines += [
            f"int {name}(CivRecomp *i,uint32_t key){{",
            f"  unsigned lo=0u,hi={len(records)}u;",
            "  while(lo<hi){unsigned mid=lo+(hi-lo)/2u;uint32_t found=records[mid].key;",
            "    if(key<found)hi=mid;else if(key>found)lo=mid+1u;",
            "    else return civ_compact_aot_execute(i,records[mid].template_id,parameters+records[mid].parameter_offset);",
            "  }",
            "  return -1;",
            "}",
            "",
        ]
        text = "\n".join(lines)
        path = shards / f"{name}.c"
        path.write_text(text, encoding="utf-8", newline="\n")
        shard_hashes[path.name] = hashlib.sha256(text.encode("utf-8")).hexdigest()

    generated_files = [out / "civilization_compact_aot_templates.c",
                       out / "civilization_compact_aot.h"] + list(shards.glob("*.c"))
    source_bytes = sum(path.stat().st_size for path in generated_files)
    manifest = {
        "format": "civilization-v35-compact-exact-context-aot-v1",
        "status": "PRODUCTION_COMPACT_EXACT_CONTEXT_AUTHORITY",
        "rom_sha256": digest,
        "analysis_sha256": sha(args.analysis),
        "indirect_proof_sha256": sha(args.indirect_proof),
        "generator_sha256": sha(Path(__file__)),
        "runtime_context_count": len(context_records),
        "semantic_template_count": len(templates),
        "parameter_word_count": parameter_words,
        "generated_shard_count": len(grouped),
        "generated_source_bytes": source_bytes,
        "generated_source_mib": round(source_bytes / 1048576, 4),
        "runtime_context_identity": "PBR:PC:E:M:X",
        "runtime_rom_opcode_decode": False,
        "runtime_learning": False,
        "runtime_emulator_fallback": False,
        "indirect_proved_site_count": len(proofs),
        "shards": shard_hashes,
        "templates_sha256": hashlib.sha256(templates_text.encode("utf-8")).hexdigest(),
        "original_semantics_sha256": original_semantics_hash.hexdigest(),
        "reconstructed_semantics_sha256": reconstructed_semantics_hash.hexdigest(),
        "all_context_semantics_reconstructed_exactly": (
            original_semantics_hash.digest() == reconstructed_semantics_hash.digest()
        ),
    }
    (out / "civilization_compact_aot.manifest.json").write_text(
        json.dumps(manifest, indent=2, sort_keys=True) + "\n",
        encoding="utf-8", newline="\n"
    )
    print(json.dumps({
        "contexts": len(context_records),
        "templates": len(templates),
        "parameter_words": parameter_words,
        "shards": len(grouped),
        "source_mib": manifest["generated_source_mib"],
    }, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
