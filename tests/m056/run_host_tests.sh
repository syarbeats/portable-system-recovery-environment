#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
gcc -std=c11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" "$ROOT/tests/m056/test_m056.c" "$ROOT/kernel/src/m056/diagnostics.c" -o "$TMP/test_m056"
"$TMP/test_m056"
echo "M056 host tests: PASS"
