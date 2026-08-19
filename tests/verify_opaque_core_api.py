from pathlib import Path
import sys


def fail(message: str) -> None:
    raise SystemExit(f"opaque core API FAIL: {message}")


root = Path(sys.argv[1]).resolve()
public = (root / "static-recomp/include/civilization_static_recomp.h").read_text(
    encoding="utf-8"
)
frontend_h = (root / "frontend/common/civilization_frontend.h").read_text(
    encoding="utf-8"
)
frontend_c = (root / "frontend/common/civilization_frontend.c").read_text(
    encoding="utf-8"
)

forward = public.find("typedef struct CivRecomp CivRecomp;")
guard = public.find("#ifdef CIVILIZATION_CORE_INTERNAL")
definition = public.find("struct CivRecomp {")
guard_end = public.find("#endif /* CIVILIZATION_CORE_INTERNAL */", definition)
if min(forward, guard, definition, guard_end) < 0:
    fail("public handle or private-layout guard is missing")
if not (forward < guard < definition < guard_end):
    fail("machine layout is not contained by the internal-build guard")
for token in ("civ_create(", "civ_destroy(", "civ_reset(", "civ_run_to_frame("):
    if token not in public:
        fail(f"public lifecycle operation missing: {token}")
if "CivRecomp *core;" not in frontend_h or "CivRecomp core;" in frontend_h:
    fail("common frontend does not own an opaque core pointer")
for token in ("f->core.", "&f->core"):
    if token in frontend_c:
        fail(f"common frontend still accesses embedded/private state: {token}")
if "civ_frontend_shutdown" not in frontend_h or "civ_destroy(f->core)" not in frontend_c:
    fail("frontend destruction does not close the core lifecycle")

print("opaque core API PASS")
