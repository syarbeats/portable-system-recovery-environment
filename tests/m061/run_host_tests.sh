#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT
cc -std=gnu11 -Wall -Wextra -Werror -I "$ROOT/kernel/include" -I "$ROOT" \
  "$ROOT/kernel/src/m061/terminal.c" "$ROOT/tests/m061/test_terminal.c" -o "$T/t"
"$T/t"
