#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
TMP="${TMPDIR:-/tmp}/psre-m046-test"
mkdir -p "$TMP"
cc -std=gnu11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" \
 "$ROOT/tests/m046/test_bot.c" "$ROOT/kernel/src/m046/bot.c" \
 -o "$TMP/test_m046"
"$TMP/test_m046"
echo "M046 BOT host tests: PASS"
