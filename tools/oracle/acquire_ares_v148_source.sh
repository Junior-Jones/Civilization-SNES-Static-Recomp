#!/usr/bin/env bash
set -euo pipefail
VERSION="v148"
ARCHIVE="ares-source.tar.gz"
SHA256="06b053d0f407d33272222efd55446d46bfd61859eea46b6ae16c0cc4fa1e51e5"
URL="https://github.com/ares-emulator/ares/releases/download/${VERSION}/${ARCHIVE}"
ROOT="${1:-$(pwd)/oracle-tools/ares-v148}"
for tool in curl sha256sum tar; do
    command -v "$tool" >/dev/null 2>&1 || { echo "missing required tool: $tool" >&2; exit 2; }
done
mkdir -p "$ROOT"
curl --fail --location --retry 3 --output "$ROOT/$ARCHIVE" "$URL"
echo "$SHA256  $ROOT/$ARCHIVE" | sha256sum --check --status || {
    echo "ares v148 source SHA-256 mismatch" >&2
    exit 3
}
rm -rf "$ROOT/source"
mkdir -p "$ROOT/source"
tar -xzf "$ROOT/$ARCHIVE" -C "$ROOT/source" --strip-components=1
printf 'ares %s source acquired and verified at %s\n' "$VERSION" "$ROOT/source"
