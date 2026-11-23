#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cc -std=c11 -Wall -Wextra -Werror -I "$ROOT/kernel/include" \
  "$ROOT/kernel/src/m052/usb_hub.c" "$ROOT/tests/m052/test_hub.c" -o /tmp/psre-m052-test
/tmp/psre-m052-test
cc -std=c11 -Wall -Wextra -Werror -ffreestanding -I "$ROOT/kernel/include" -fsyntax-only \
  "$ROOT/kernel/src/m052/usb_hub.c" "$ROOT/kernel/src/main.c" "$ROOT/kernel/src/m006/cli.c"
grep -q 'test-m052' "$ROOT/Makefile"
echo '[PASS] M052 freestanding integration syntax'
