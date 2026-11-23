#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
OUT="${TMPDIR:-/tmp}/psre_m036_test"
cc -std=gnu11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" \
  "$ROOT/kernel/src/m036/app.c" "$ROOT/tests/m036/test_app.c" -o "$OUT"
"$OUT"
rm -f "$OUT"
