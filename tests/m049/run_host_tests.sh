#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
cc -std=c11 -Wall -Wextra -Werror -ffreestanding -I "$ROOT/kernel/include" \
  -DPSRE_M049_ENABLE_AUTOSTART=0 -fsyntax-only "$ROOT/kernel/src/m049/usb_gui.c"
grep -q 'psre_m049_initialize' "$ROOT/kernel/src/main.c"
grep -q 'm049-poll' "$ROOT/kernel/src/m006/cli.c"
grep -q 'PSRE_M049_ENABLE_AUTOSTART' "$ROOT/kernel/src/m049/usb_gui.c"
echo '[PASS] M049 safe-default integration syntax and boot/CLI wiring'
