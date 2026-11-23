#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
gcc -std=c11 -Wall -Wextra -Werror -I"$ROOT/kernel/include" \
  "$ROOT/tests/m055/test_m055.c" "$ROOT/kernel/src/m055/input_service.c" \
  -o "$TMP/test_m055"
"$TMP/test_m055"
echo "M055 host tests: PASS"
