#!/usr/bin/env bash
set -euo pipefail

PRESET="${1:-default}"
BUILD_DIR="build/debug"
[[ "$PRESET" == "release" ]] && BUILD_DIR="build/release"

cmake --preset "$PRESET" -Wno-dev > /dev/null 2>&1
cmake --build --preset "$PRESET"
exec "./$BUILD_DIR/PWDoom"
