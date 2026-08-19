#!/usr/bin/env python3
"""Emit one experimental bank as conservative multi-instruction blocks.

The output is intentionally not wired into production.  It reuses the exact
V33 semantic emitter and finite return/indirect proof tables, but dispatches
only at block entries.  This allows source-size and compiler validation before
the runtime authority is changed.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from collections import defaultdict
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent / "rom"))
from civilization_rom import EXPECTED_SHA, EXPECTED_SIZE, HiRom  # noqa: E402
from generate_v33_closed_core import (  # noqa: E402
    ctx_key,
    emit_case,
    load_proofs,
    make_return_proofs,
    parse_addr,
)
from w65c816 import CpuContext, decode  # noqa: E402


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("rom", type=Path)
    ap.add_argument("--analysis", type=Path, required=True)
    ap.add_argument("--inventory", type=Path, required=True)
    ap.add_argument("--indirect-proof", type=Path, required=True)
    ap.add_argument("--bank", default="C0")
    ap.add_argument("--out-c", type=Path, required=True)
    ap.add_argument("--out-shard-dir", type=Path)
    ap.add_argument("--generated-index", type=Path)
    ap.add_argument("--out-non-bank-index", type=Path)
    ap.add_argument("--out-block-index", type=Path)
    ap.add_argument("--out-json", type=Path, required=True)
    args = ap.parse_args()

    raw = args.rom.read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    if len(raw) != EXPECTED_SIZE or digest != EXPECTED_SHA:
        raise SystemExit("wrong Civilization ROM")
    analysis = json.loads(args.analysis.read_text(encoding="utf-8"))
    inventory = json.loads(args.inventory.read_text(encoding="utf-8"))
    if analysis.get("rom_sha256") != digest:
        raise SystemExit("analysis/ROM mismatch")
    bank = int(args.bank, 16)
    rom = HiRom(raw)
    _proof_doc, proofs = load_proofs(args.indirect_proof)
    _sets, ret_ids, _pool, _meta = make_return_proofs(analysis, rom)

    rows = defaultdict(dict)
    for row in analysis.get("contexts", []):
        b, pc = parse_addr(row["address"])
        if b != bank:
            continue
        key = (int(row["e"]), int(row["m"]), int(row["x"]))
        rows[pc][key] = row

    emitted = []
    block_count = 0
    entry_context_count = 0
    emitted_instruction_bodies = 0
    for block in inventory.get("blocks", []):
        entry_bank, entry_pc = parse_addr(block["entry"])
        if entry_bank != bank:
            continue
        last_bank, last_pc = parse_addr(block["last_instruction"])
        if last_bank != bank:
            raise RuntimeError("cross-bank block in conservative inventory")
        block_count += 1
        pcs = []
        pc = entry_pc
        while True:
            pcs.append(pc)
            if pc == last_pc:
                break
            candidates = list(rows.get(pc, {}).values())
            if not candidates:
                raise RuntimeError(f"missing block instruction at {bank:02X}:{pc:04X}")
            pc = (pc + len(bytes.fromhex(candidates[0]["bytes"]))) & 0xFFFF

        for width in sorted(rows[entry_pc]):
            e, m, x = width
            steps = []
            for index, instruction_pc in enumerate(pcs):
                row = rows.get(instruction_pc, {}).get(width)
                if row is None:
                    raise RuntimeError(
                        f"block width vanished at {bank:02X}:{instruction_pc:04X} E{e}M{m}X{x}"
                    )
                context = CpuContext(bank, instruction_pc, e, m, x, None)
                inst = decode(rom.fetch, context)
                body = emit_case(context, inst, proofs, ret_ids)[1:]
                if index + 1 < len(pcs):
                    if body[-1].strip() != "i->instruction_count++;return 1;":
                        raise RuntimeError(
                            f"non-final instruction unexpectedly returns at {bank:02X}:{instruction_pc:04X}"
                        )
                    body[-1] = "    i->instruction_count++;"
                    next_pc = pcs[index + 1]
                    body.append(
                        f"    if(!civ_v35_block_continue(i,0x{bank:02X}u,0x{next_pc:04X}u))return !i->failed;"
                    )
                steps.append({
                    "key": ctx_key(context),
                    "address": f"{bank:02X}:{instruction_pc:04X}",
                    "width": width,
                    "body": tuple(body),
                })
                emitted_instruction_bodies += 1
            entry_context_count += 1
            group = (((bank << 16) | entry_pc) >> 10)
            if any(((step["key"] & 0x00FFFFFF) >> 10) != group for step in steps):
                raise RuntimeError(
                    f"block crosses dispatch shard at {block['entry']}; regenerate inventory"
                )
            emitted.append({
                "entry": block["entry"],
                "width": width,
                "group": group,
                "steps": steps,
            })

    identical = defaultdict(list)
    for block in emitted:
        signature = tuple(step["body"] for step in block["steps"])
        identical[signature].append(block["entry"])

    def emit_function(function_name: str, blocks: list[dict]) -> str:
        output = [
            "/* EXPERIMENTAL: generated V35 conservative basic-block code. */",
            "/* Every proved context is retained as an exact resume entry. */",
            '#include "civilization_internal.h"',
            '#include "civilization_generated_core.h"',
            "",
            f"int {function_name}(CivRecomp *i,uint32_t key){{",
            "  if(!i)return 0;",
            "  switch(key){",
        ]
        for compact_block in blocks:
            for step in compact_block["steps"]:
                output.append(
                    f"  case 0x{step['key']:08X}u: /* {step['address']} "
                    f"E{step['width'][0]}M{step['width'][1]}X{step['width'][2]} */"
                )
                output.extend(step["body"])
        output += ["  default:return -1;", "  }", "}", ""]
        return "\n".join(output)

    text = emit_function(f"civ_v35_block_bank_{bank:02x}", emitted)
    args.out_c.parent.mkdir(parents=True, exist_ok=True)
    args.out_json.parent.mkdir(parents=True, exist_ok=True)
    args.out_c.write_text(text, encoding="utf-8", newline="\n")

    shard_files = []
    shard_source_bytes = 0
    by_group = defaultdict(list)
    for block in emitted:
        by_group[block["group"]].append(block)
    if args.out_shard_dir:
        args.out_shard_dir.mkdir(parents=True, exist_ok=True)
        for group, group_blocks in sorted(by_group.items()):
            function_name = f"civ_v35_block_group_{group:05x}"
            shard_text = emit_function(function_name, group_blocks)
            shard_path = args.out_shard_dir / f"civilization_block_group_{group:05X}.c"
            shard_path.write_text(shard_text, encoding="utf-8", newline="\n")
            shard_files.append(shard_path)
            shard_source_bytes += len(shard_text.encode("utf-8"))

    if args.out_block_index:
        index_lines = [
            "/* EXPERIMENTAL: generated V35 compact-bank dispatch index. */",
            '#include "civilization_internal.h"',
            '#include "civilization_generated_core.h"',
            "",
        ]
        for group in sorted(by_group):
            index_lines.append(f"int civ_v35_block_group_{group:05x}(CivRecomp*,uint32_t);")
        index_lines += [
            "",
            f"int civ_v35_block_bank_{bank:02x}_step(CivRecomp *i){{",
            "  uint32_t key,group;int result;if(!i)return 0;",
            "  key=civ_generated_core_context_key(i);group=(key&0x00FFFFFFu)>>10u;",
            "  switch(group){",
        ]
        for group in sorted(by_group):
            index_lines.append(
                f"  case 0x{group:X}u:result=civ_v35_block_group_{group:05x}(i,key);break;"
            )
        index_lines += ["  default:return -1;", "  }", "  return result;", "}", ""]
        args.out_block_index.parent.mkdir(parents=True, exist_ok=True)
        args.out_block_index.write_text("\n".join(index_lines), encoding="utf-8", newline="\n")

    if args.generated_index or args.out_non_bank_index:
        if not args.generated_index or not args.out_non_bank_index:
            raise SystemExit("--generated-index and --out-non-bank-index must be used together")
        group_min = ((bank << 16) >> 10)
        group_max = (((bank << 16) | 0xFFFF) >> 10)
        prototype_re = re.compile(r"^int civilization_core_group_([0-9A-Fa-f]{5})\(")
        case_re = re.compile(r"^\s*case 0x([0-9A-Fa-f]+)u:result=civilization_core_group_")
        filtered = ["/* Generated V35 test index with the compacted bank removed. */"]
        for source_line in args.generated_index.read_text(encoding="utf-8").splitlines():
            match = prototype_re.match(source_line) or case_re.match(source_line)
            if match and group_min <= int(match.group(1), 16) <= group_max:
                continue
            filtered.append(source_line)
        args.out_non_bank_index.parent.mkdir(parents=True, exist_ok=True)
        args.out_non_bank_index.write_text(
            "\n".join(filtered) + "\n", encoding="utf-8", newline="\n"
        )

    existing = list((args.out_c.parents[2] / "static-recomp" / "generated" / "civilization_generated_core_shards").glob("*.c"))
    group_min = ((bank << 16) >> 10)
    group_max = (((bank << 16) | 0xFFFF) >> 10)
    existing_bank_files = []
    for path in existing:
        try:
            group = int(path.stem.rsplit("_", 1)[1], 16)
        except ValueError:
            continue
        if group_min <= group <= group_max:
            existing_bank_files.append(path)
    old_bytes = sum(path.stat().st_size for path in existing_bank_files)
    report = {
        "format": "civilization-v35-basic-block-prototype-v1",
        "status": "EXPERIMENTAL_TEST_OPTION",
        "bank": f"{bank:02X}",
        "rom_sha256": digest,
        "analysis_sha256": hashlib.sha256(args.analysis.read_bytes()).hexdigest(),
        "inventory_sha256": hashlib.sha256(args.inventory.read_bytes()).hexdigest(),
        "block_count": block_count,
        "entry_context_count": entry_context_count,
        "resume_context_count": emitted_instruction_bodies,
        "emitted_instruction_bodies": emitted_instruction_bodies,
        "identical_block_body_groups": sum(1 for entries in identical.values() if len(entries) > 1),
        "folded_block_entry_contexts": 0,
        "prototype_source_bytes": len(text.encode("utf-8")),
        "prototype_shard_count": len(shard_files),
        "prototype_sharded_source_bytes": shard_source_bytes,
        "current_bank_shard_count": len(existing_bank_files),
        "current_bank_source_bytes": old_bytes,
        "source_byte_change": len(text.encode("utf-8")) - old_bytes,
        "source_reduction_percent": round(
            (old_bytes - len(text.encode("utf-8"))) * 100.0 / old_bytes, 4
        ) if old_bytes else None,
        "sharded_source_reduction_percent": round(
            (old_bytes - shard_source_bytes) * 100.0 / old_bytes, 4
        ) if old_bytes and shard_source_bytes else None,
        "runtime_decoder": False,
        "runtime_learning": False,
        "runtime_fallback": False,
    }
    args.out_json.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
