#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
grep -Fq 'LIMINE_COMMON_MAGIC_0, LIMINE_COMMON_MAGIC_1' "$ROOT/kernel/include/psre/limine.h"
grep -Fq '0x9d5827dcd881dd75ULL, 0xa3148604f6fab11bULL' "$ROOT/kernel/include/psre/limine.h"
grep -Fq '0x785c6ed015d3e316ULL, 0x181e920a7852b9d9ULL' "$ROOT/kernel/src/limine_requests.c"
grep -Fq 'limine_requests_start_marker[4]' "$ROOT/kernel/src/limine_requests.c"
grep -Fq 'PSRE Integrated Build 4.9.3.1' "$ROOT/kernel/src/main.c"
echo 'PASS: Limine framebuffer request ID and request delimiters match protocol constants.'
