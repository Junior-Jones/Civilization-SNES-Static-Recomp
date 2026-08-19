from pathlib import Path
import sys


def fail(message: str) -> None:
    raise SystemExit(f"diagnostics API separation FAIL: {message}")


root = Path(sys.argv[1]).resolve()
stable = (root / "static-recomp/include/civilization_static_recomp.h").read_text(encoding="utf-8")
diagnostics = (root / "static-recomp/include/civilization_diagnostics.h").read_text(encoding="utf-8")

for token in (
    "CivV20AudioStatus", "CivVideoCheckpoint", "CivDiagnosticState",
    "civ_diagnostics_capture", "civ_diagnostics_read_wram",
    "civ_v20_get_audio_status", "civ_v20_audio_peek_dsp",
    "civ_v20_audio_peek_aram", "civ_v20_write_wav",
):
    if token in stable:
        fail(f"stable header still exports {token}")
    if token not in diagnostics:
        fail(f"diagnostics header is missing {token}")

for relative in (
    "frontend/common/civilization_frontend.c",
    "frontend/common/civilization_frontend.h",
    "frontend/linux/civilization_headless.c",
    "frontend/windows/civilization_app_core.c",
):
    text = (root / relative).read_text(encoding="utf-8")
    if "civilization_internal.h" in text:
        fail(f"frontend bypasses diagnostics API: {relative}")

if "civilization_diagnostics.h" not in (root / "frontend/linux/civilization_headless.c").read_text(encoding="utf-8"):
    fail("headless research frontend does not opt into diagnostics")
if "civilization_diagnostics.h" not in (root / "frontend/windows/civilization_app_core.c").read_text(encoding="utf-8"):
    fail("Windows diagnostic writer does not opt into diagnostics")

print("diagnostics API separation PASS")
