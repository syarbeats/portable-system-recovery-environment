#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cc -std=c11 -Wall -Wextra -Werror -I "$ROOT/kernel/include" \
  "$ROOT/kernel/src/m053/hub_driver.c" "$ROOT/kernel/src/m052/usb_hub.c" \
  "$ROOT/tests/m053/test_hub_driver.c" -o /tmp/psre-m053-test
/tmp/psre-m053-test
cc -std=c11 -Wall -Wextra -Werror -ffreestanding -I "$ROOT/kernel/include" -fsyntax-only \
  "$ROOT/kernel/src/m053/hub_driver.c" "$ROOT/kernel/src/main.c" "$ROOT/kernel/src/m006/cli.c"
grep -q 'test-m053' "$ROOT/Makefile"
echo '[PASS] M053 freestanding integration syntax'
