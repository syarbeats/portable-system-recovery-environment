#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${TMPDIR:-/tmp}/psre-m032-host-test"
cc -std=gnu11 -O1 -g -Wall -Wextra -Werror -I"$ROOT/kernel/include" \
  "$ROOT/kernel/src/m031/framebuffer.c" \
  "$ROOT/kernel/src/m032/graphics.c" \
  "$ROOT/tests/m032/test_graphics.c" -o "$OUT"
"$OUT"
