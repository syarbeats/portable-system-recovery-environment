#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cc -std=c11 -Wall -Wextra -Werror -ffreestanding -I "$ROOT/kernel/include" -fsyntax-only \
  "$ROOT/kernel/src/m051/device_lifecycle.c" "$ROOT/kernel/src/m016/usb_ports.c"
cc -std=c11 -Wall -Wextra -Werror -ffreestanding -I "$ROOT/kernel/include" -fsyntax-only \
  "$ROOT/kernel/src/main.c" "$ROOT/kernel/src/m006/cli.c" "$ROOT/kernel/src/m050/event_loop.c"
grep -q 'psre_m051_scan' "$ROOT/kernel/src/m050/event_loop.c"
grep -q 'test-m051' "$ROOT/Makefile"
echo '[PASS] M051 lifecycle monitor, PORTSC read-only sampling and integration syntax'
