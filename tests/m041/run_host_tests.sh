#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
OUT="${TMPDIR:-/tmp}/psre_m041_test"
cc -std=gnu11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" "$ROOT/kernel/src/m041/recovery_manager.c" "$ROOT/tests/m041/test_recovery_manager.c" -o "$OUT"
"$OUT"
rm -f "$OUT"
