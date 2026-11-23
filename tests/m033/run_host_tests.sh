#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
OUT=$(mktemp)
trap 'rm -f "$OUT"' EXIT
cc -std=c11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" \
  "$ROOT/kernel/src/m033/font.c" "$ROOT/tests/m033/test_font.c" -o "$OUT"
"$OUT"
