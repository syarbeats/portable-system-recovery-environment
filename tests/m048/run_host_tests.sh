#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
cc -std=c11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" -fsyntax-only "$ROOT/kernel/src/m048/boot.c"
echo "[PASS] M048 boot coordinator strict host syntax check"
