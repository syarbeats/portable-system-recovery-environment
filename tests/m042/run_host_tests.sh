#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
OUT="${TMPDIR:-/tmp}/psre_m042_test"
cc -std=gnu11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" "$ROOT/kernel/src/m042/file_explorer.c" "$ROOT/tests/m042/test_file_explorer.c" -o "$OUT"
"$OUT"
rm -f "$OUT"
