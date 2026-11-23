#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
OUT="${TMPDIR:-/tmp}/psre_m039_test"
cc -std=gnu11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" \
  "$ROOT/kernel/src/m038/container.c" "$ROOT/kernel/src/m039/layout.c" \
  "$ROOT/tests/m039/test_layout.c" -o "$OUT"
"$OUT"
rm -f "$OUT"
