#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
OUT="${TMPDIR:-/tmp}/psre_m038_test"
cc -std=gnu11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" \
  "$ROOT/kernel/src/m038/container.c" "$ROOT/tests/m038/test_container.c" -o "$OUT"
"$OUT"
rm -f "$OUT"
