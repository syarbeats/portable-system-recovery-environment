#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${TMPDIR:-/tmp}/psre-m031-framebuffer-test"
cc -std=gnu11 -O1 -g -Wall -Wextra -Werror -I"$ROOT/kernel/include" \
  "$ROOT/kernel/src/m031/framebuffer.c" "$ROOT/tests/m031/test_framebuffer.c" -o "$OUT"
"$OUT"
