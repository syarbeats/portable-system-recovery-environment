#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
OUT="${TMPDIR:-/tmp}/psre_m034_test"
cc -std=gnu11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" \
  "$ROOT/kernel/src/m034/window.c" "$ROOT/tests/m034/test_window.c" -o "$OUT"
"$OUT"
rm -f "$OUT"
