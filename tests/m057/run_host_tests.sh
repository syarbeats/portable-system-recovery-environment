#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
gcc -std=c11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" "$ROOT/tests/m057/test_m057.c" "$ROOT/kernel/src/m057/diagnostics.c" -o "$TMP/test_m057"
"$TMP/test_m057"
echo "M057 host tests: PASS"
