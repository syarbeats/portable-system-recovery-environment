#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
OUT="${TMPDIR:-/tmp}/psre_m043_test"
cc -std=gnu11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" "$ROOT/kernel/src/m043/process.c" "$ROOT/tests/m043/test_process.c" -o "$OUT"
"$OUT"
rm -f "$OUT"
