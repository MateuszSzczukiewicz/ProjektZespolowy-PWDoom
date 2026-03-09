#!/usr/bin/env bash
set -euo pipefail

PRESET="${1:-default}"
BUILD_DIR="build/debug"
[[ "$PRESET" == "release" ]] && BUILD_DIR="build/release"

cmake --preset "$PRESET" -Wno-dev > /dev/null 2>&1

while true; do
    find src/ include/ -name '*.c' -o -name '*.h' | \
        entr -d -r sh -c "cmake --build --preset $PRESET && ./$BUILD_DIR/PWDoom"
done
