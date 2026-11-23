#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
TMP="${TMPDIR:-/tmp}/psre-m047-test"; mkdir -p "$TMP"
cc -std=gnu11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" "$ROOT/tests/m047/test_block.c" "$ROOT/kernel/src/m047/block.c" -o "$TMP/test_m047"
"$TMP/test_m047"
echo "M047 USB block device tests: PASS"
