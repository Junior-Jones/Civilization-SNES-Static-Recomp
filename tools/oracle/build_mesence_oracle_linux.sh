#!/usr/bin/env bash
set -euo pipefail
if [[ $# -ne 1 ]]; then
    echo "usage: $0 /path/to/MesenCE-source" >&2
    exit 2
fi
SOURCE="$1"
if [[ ! -f "$SOURCE/makefile" && ! -f "$SOURCE/Makefile" ]]; then
    echo "not a MesenCE source tree: $SOURCE" >&2
    exit 3
fi
command -v clang >/dev/null 2>&1 || { echo "clang is required" >&2; exit 4; }
command -v dotnet >/dev/null 2>&1 || { echo ".NET 8 SDK is required by upstream MesenCE Linux build" >&2; exit 5; }
if command -v pkg-config >/dev/null 2>&1; then
    pkg-config --exists sdl2 || { echo "SDL2 development files are required by upstream MesenCE Linux build" >&2; exit 6; }
elif ! command -v sdl2-config >/dev/null 2>&1; then
    echo "SDL2 development files are required by upstream MesenCE Linux build" >&2
    exit 6
fi
make -C "$SOURCE"
