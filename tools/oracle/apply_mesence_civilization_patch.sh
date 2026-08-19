#!/usr/bin/env bash
set -euo pipefail
if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "usage: $0 /path/to/MesenCE-2.2.1-source [/path/to/project/oracle-patches/mesence-2.2.1]" >&2
    exit 2
fi
MESEN=$1
if [[ $# -eq 2 ]]; then
    KIT=$2
else
    ROOT=$(cd "$(dirname "$0")/../.." && pwd)
    KIT="$ROOT/oracle-patches/mesence-2.2.1"
fi
EXPECTED_BLOB="9189d58981883c977a6af0c8f190b97fa8e23d0e"
TARGET="$MESEN/Core/SNES/Debugger/SnesDebugger.cpp"
PATCH="$KIT/civilization-mesence-oracle.patch"
if [[ ! -f "$TARGET" ]]; then
    echo "missing MesenCE target file: $TARGET" >&2
    exit 3
fi
if [[ ! -f "$PATCH" ]]; then
    echo "missing Civilization oracle patch: $PATCH" >&2
    exit 6
fi
actual_blob=$(python3 - "$TARGET" <<'PY'
from pathlib import Path
import hashlib,sys
b=Path(sys.argv[1]).read_bytes()
print(hashlib.sha1(b"blob "+str(len(b)).encode()+b"\0"+b).hexdigest())
PY
)
if [[ "$actual_blob" != "$EXPECTED_BLOB" ]]; then
    echo "unexpected SnesDebugger.cpp blob: $actual_blob" >&2
    exit 4
fi
if [[ -e "$MESEN/Core/SNES/Debugger/CivilizationOracleTrace.h" ]]; then
    echo "oracle source is not pristine: CivilizationOracleTrace.h already exists" >&2
    exit 5
fi
patch --dry-run -d "$MESEN" -p1 < "$PATCH" >/dev/null
patch -d "$MESEN" -p1 < "$PATCH"
printf 'Civilization Version 08 oracle instrumentation applied to verified MesenCE 2.2.1 source\n'
