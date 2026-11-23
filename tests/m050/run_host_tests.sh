#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cc -std=c11 -Wall -Wextra -Werror -ffreestanding -I "$ROOT/kernel/include" -fsyntax-only \
  "$ROOT/kernel/src/m050/event_loop.c" "$ROOT/kernel/src/m050/device_manager.c"
cc -std=c11 -Wall -Wextra -Werror -ffreestanding -I "$ROOT/kernel/include" -fsyntax-only \
  "$ROOT/kernel/src/serial.c" "$ROOT/kernel/src/m006/cli.c" "$ROOT/kernel/src/main.c"
grep -q 'psre_m050_run_once' "$ROOT/kernel/src/m006/cli.c"
grep -q 'psre_m050_initialize' "$ROOT/kernel/src/main.c"
grep -q 'test-m050' "$ROOT/Makefile"
echo '[PASS] M050 scheduler, device manager, serial cooperative loop and integration syntax'
