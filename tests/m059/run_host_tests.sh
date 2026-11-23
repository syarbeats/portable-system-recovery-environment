#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT
cc -std=gnu11 -Wall -Wextra -Werror -ffreestanding -I "$ROOT/kernel/include" -I "$ROOT" \
  "$ROOT/kernel/src/m022/interrupt.c" "$ROOT/tests/m059/test_m022_idle_wrap.c" -o "$T/t" 2>&1 | grep -v "multiple definition" || true
"$T/t"
