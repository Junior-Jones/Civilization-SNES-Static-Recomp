#!/usr/bin/env python3
"""Keep superseded hybrid audio state out of the production runtime."""

from __future__ import annotations

import argparse
from pathlib import Path


FORBIDDEN = (
    "CivSmpState",
    "CivApuTraceEvent",
    "CIV_V13_APU_TRACE_CAPACITY",
    "CIV_V14_COMMAND_COUNT",
    "CIV_ARAM_KNOWN_MASK_SIZE",
    "CIV_SDSP_KNOWN_MASK_SIZE",
    "apu_cpu_to_smp",
    "apu_smp_to_cpu",
    "apu_cpu_write_count",
    "apu_cpu_read_count",
    "apu_smp_write_count",
    "apu_smp_read_count",
    "aram_known",
    "smp_unknown_read_count",
    "sdsp_known",
    "sdsp_register_write_count",
    "v11_",
    "v12_smp_rendezvous_complete",
    "v13_",
    "v14_",
    "v15_wram_clear_bytes",
    "v16_command_",
    "v16_selector",
    "v16_aram_poweron",
    "v19_rendezvous_enabled",
    "v19_selector",
    "v19_upload_",
    "v19_command_",
    "v19_apu_event_",
    "v19_audio_return_observed",
    "v20_apu_read_compare_count",
    "v20_apu_read_mismatch_count",
    "cpu_read_compare_count",
    "cpu_read_mismatch_count",
)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("project", type=Path)
    args = parser.parse_args()
    root = args.project.resolve()
    production = (
        root / "static-recomp" / "include" / "civilization_static_recomp.h",
        root / "static-recomp" / "src" / "civilization_audio.cpp",
        root / "static-recomp" / "src" / "civilization_bus.c",
        root / "static-recomp" / "src" / "civilization_machine.c",
        root / "static-recomp" / "src" / "civilization_runtime.c",
    )
    missing = [str(path.relative_to(root)) for path in production if not path.is_file()]
    if missing:
        raise RuntimeError(f"missing production files: {missing}")
    combined = "\n".join(path.read_text(encoding="utf-8") for path in production)
    found = [token for token in FORBIDDEN if token in combined]
    required = (
        "v20_full_static_audio_authoritative",
        "sc_static_apu_acquire",
        "sc_static_apu_sync_to_master",
        "civ_v20_audio_cpu_write",
        "civ_v20_audio_cpu_read",
    )
    absent = [token for token in required if token not in combined]
    if found or absent:
        raise RuntimeError(f"legacy tokens={found}; missing Full Static tokens={absent}")
    print("PASS: production state contains only the authoritative Full Static audio lane")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
