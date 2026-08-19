#!/usr/bin/env python3
"""Verify the promoted Version 35 compact exact-context AOT authority."""
from __future__ import annotations

import argparse
import hashlib
import json
import sys
from pathlib import Path


ROM_SHA = "de2d5a952096c5f50368b9270d342aa6e7a39007ffbec27117e182e30ef4cf32"
SEMANTICS_SHA = "144dcfc5b91247bd22a227b8a274d5b1e8a4cac5abe6367762f5a6c219e9b700"
CONTEXTS = 103584
TEMPLATES = 299
PARAMETER_WORDS = 294241
SHARDS = 280


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--project", type=Path, required=True)
    args = parser.parse_args()
    root = args.project.resolve()
    authority = root / "static-recomp" / "generated" / "compact-aot"
    manifest_path = authority / "civilization_compact_aot.manifest.json"
    templates_path = authority / "civilization_compact_aot_templates.c"
    header_path = authority / "civilization_compact_aot.h"
    shard_dir = authority / "civilization_compact_aot_shards"
    cmake_path = root / "static-recomp" / "CMakeLists.txt"
    generator_path = root / "tools" / "scpu" / "generate_v35_compact_aot_core.py"
    required = (manifest_path, templates_path, header_path, cmake_path,
                generator_path)
    missing = [str(path.relative_to(root)) for path in required
               if not path.is_file()]
    if missing:
        raise RuntimeError(f"missing compact authority files: {missing}")

    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    receipts = manifest.get("shards", {})
    shards = sorted(shard_dir.glob("*.c"))
    semantics_original = manifest.get("original_semantics_sha256")
    semantics_reconstructed = manifest.get("reconstructed_semantics_sha256")
    cmake = cmake_path.read_text(encoding="utf-8")
    source_bytes = (templates_path.stat().st_size + header_path.stat().st_size +
                    sum(path.stat().st_size for path in shards))

    checks = {
        "format": manifest.get("format") ==
                  "civilization-v35-compact-exact-context-aot-v1",
        "rom_identity": manifest.get("rom_sha256") == ROM_SHA,
        "exact_context_identity": manifest.get("runtime_context_identity") ==
                                  "PBR:PC:E:M:X",
        "context_count": manifest.get("runtime_context_count") == CONTEXTS,
        "template_count": manifest.get("semantic_template_count") == TEMPLATES,
        "parameter_word_count": manifest.get("parameter_word_count") ==
                                PARAMETER_WORDS,
        "manifest_shard_count": manifest.get("generated_shard_count") == SHARDS,
        "physical_shard_count": len(shards) == SHARDS,
        "receipt_count": len(receipts) == SHARDS,
        "all_shard_hashes": all(
            (shard_dir / name).is_file() and
            sha256(shard_dir / name) == digest
            for name, digest in receipts.items()),
        "templates_hash": manifest.get("templates_sha256") ==
                          sha256(templates_path),
        "generator_hash": manifest.get("generator_sha256") ==
                          sha256(generator_path),
        "generated_source_bytes": manifest.get("generated_source_bytes") ==
                                  source_bytes,
        "exact_reconstruction_asserted":
            manifest.get("all_context_semantics_reconstructed_exactly") is True,
        "exact_reconstruction_hash": semantics_original == SEMANTICS_SHA and
                                     semantics_reconstructed == SEMANTICS_SHA,
        "no_runtime_opcode_decode":
            manifest.get("runtime_rom_opcode_decode") is False,
        "no_runtime_learning": manifest.get("runtime_learning") is False,
        "no_runtime_emulator_fallback":
            manifest.get("runtime_emulator_fallback") is False,
        "compact_is_only_cmake_authority":
            "generated/compact-aot" in cmake and
            "civilization_compact_aot_templates.c" in cmake and
            "civilization_generated_core_shards" not in cmake and
            "option(" not in cmake,
        "superseded_body_shards_absent": not (
            root / "static-recomp" / "generated" /
            "civilization_generated_core_shards").exists(),
    }
    failed = [name for name, passed in checks.items() if not passed]
    report = {
        "pass": not failed,
        "format": manifest.get("format"),
        "runtime_contexts": manifest.get("runtime_context_count"),
        "semantic_templates": manifest.get("semantic_template_count"),
        "parameter_words": manifest.get("parameter_word_count"),
        "shards": len(shards),
        "generated_source_bytes": source_bytes,
        "semantics_sha256": semantics_reconstructed,
        "checks": checks,
        "failed_checks": failed,
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if not failed else 1


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as exc:
        print(json.dumps({"pass": False, "error": str(exc)}, indent=2))
        sys.exit(1)
