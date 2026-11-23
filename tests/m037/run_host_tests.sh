#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
OUT="${TMPDIR:-/tmp}/psre_m037_test"
cc -std=gnu11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" "$ROOT/kernel/src/m037/widget.c" "$ROOT/tests/m037/test_widget.c" -o "$OUT"
"$OUT"
rm -f "$OUT"
