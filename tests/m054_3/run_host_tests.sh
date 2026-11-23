#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
grep -q 'KEEP(\*(.limine_requests_start))' "$ROOT/arch/x86_64/linker.ld"
grep -q 'KEEP(\*(.limine_requests))' "$ROOT/arch/x86_64/linker.ld"
grep -q 'KEEP(\*(.limine_requests_end))' "$ROOT/arch/x86_64/linker.ld"
grep -q 'LIMINE_FRAMEBUFFER_REQUEST_ID' "$ROOT/kernel/src/limine_requests.c"
grep -q 'LIMINE_FRAMEBUFFER_REQUEST_REVISION' "$ROOT/kernel/src/limine_requests.c"
grep -q 'Response: ' "$ROOT/kernel/src/m031/framebuffer.c"
echo "PASS: M054.3 Limine request registration source checks."
