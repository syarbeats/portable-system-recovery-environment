#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT
cc -std=gnu11 -Wall -Wextra -Werror -I "$ROOT/kernel/include" -I "$ROOT" \
  "$ROOT/kernel/src/m031/framebuffer.c" "$ROOT/tests/m060/test_m031_backbuffer.c" -o "$T/t"
"$T/t"
