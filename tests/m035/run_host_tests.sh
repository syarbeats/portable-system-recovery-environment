#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
OUT="${TMPDIR:-/tmp}/psre_m035_test"
cc -std=gnu11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" \
  "$ROOT/kernel/src/m035/shell.c" "$ROOT/kernel/src/m036/app.c" "$ROOT/kernel/src/m037/widget.c" "$ROOT/kernel/src/m038/container.c" "$ROOT/kernel/src/m039/layout.c" "$ROOT/kernel/src/m040/controls.c" "$ROOT/tests/m035/test_shell.c" -o "$OUT"
"$OUT"
rm -f "$OUT"
