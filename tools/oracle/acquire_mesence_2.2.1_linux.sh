#!/usr/bin/env bash
set -euo pipefail

# Acquire the exact Linux oracle binary and exact editable source tree selected
# for the Civilization static-recomp project.  This script is developer-only;
# neither the binary nor MesenCE source is linked into the static recomp.

TAG="2.2.1"
COMMIT="20ba206cef5ba207c21203176d02cb9f43dda9fb"
BINARY_NAME="Mesen_2.2.1_Linux_x64.zip"
BINARY_SHA256="c88ff4d251b407515c43d3332d641927655cd69fb538996b6a21da4509dbb58f"
REPO="https://github.com/nesdev-org/MesenCE.git"
BINARY_URL="https://github.com/nesdev-org/MesenCE/releases/download/${TAG}/${BINARY_NAME}"
ROOT="${1:-$(pwd)/oracle-tools/mesence-2.2.1}"
PATCH_KIT="${2:-}"

need() { command -v "$1" >/dev/null 2>&1 || { echo "missing required tool: $1" >&2; exit 2; }; }
need git
need curl
need sha256sum
need unzip

mkdir -p "$ROOT"

if [[ ! -d "$ROOT/source/.git" ]]; then
    rm -rf "$ROOT/source"
    git clone --no-tags "$REPO" "$ROOT/source"
fi

git -C "$ROOT/source" fetch --force origin "refs/tags/${TAG}:refs/tags/${TAG}"
git -C "$ROOT/source" checkout --detach "$COMMIT"
actual_commit="$(git -C "$ROOT/source" rev-parse HEAD)"
if [[ "$actual_commit" != "$COMMIT" ]]; then
    echo "source commit mismatch: $actual_commit" >&2
    exit 3
fi

if [[ -n "$PATCH_KIT" ]]; then
    "$(dirname "$0")/apply_mesence_civilization_patch.sh" "$ROOT/source" "$PATCH_KIT"
fi

curl --fail --location --retry 3 --output "$ROOT/$BINARY_NAME" "$BINARY_URL"
echo "$BINARY_SHA256  $ROOT/$BINARY_NAME" | sha256sum --check --status || {
    echo "MesenCE Linux binary SHA-256 mismatch" >&2
    exit 4
}
rm -rf "$ROOT/binary"
mkdir -p "$ROOT/binary"
unzip -q "$ROOT/$BINARY_NAME" -d "$ROOT/binary"

{
    echo "MesenCE oracle acquisition receipt"
    echo "tag=$TAG"
    echo "commit=$actual_commit"
    echo "binary=$BINARY_NAME"
    echo "binary_sha256=$BINARY_SHA256"
    echo "source_tree=$ROOT/source"
    echo "binary_tree=$ROOT/binary"
    if [[ -n "$PATCH_KIT" ]]; then echo "oracle_patch_kit=$PATCH_KIT"; fi
} > "$ROOT/ACQUISITION-RECEIPT.txt"

printf 'MesenCE %s acquired and verified at %s\n' "$TAG" "$ROOT"
