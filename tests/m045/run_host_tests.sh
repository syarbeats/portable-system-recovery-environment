#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
OUT="${TMPDIR:-/tmp}/psre_m045_test"
cc -std=gnu11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" "$ROOT/kernel/src/m045/bulk.c" "$ROOT/tests/m045/test_bulk.c" -o "$OUT"
"$OUT"
rm -f "$OUT"
echo "M045 host tests: PASS"
