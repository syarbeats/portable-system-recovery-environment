#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
# Hardware implementation is syntax-checked; runtime registration requires M015 DMA/MMIO.
cc -std=c11 -Wall -Wextra -Werror -ffreestanding -I "$ROOT/kernel/include" -fsyntax-only \
 "$ROOT/kernel/src/m054/xhci_slots.c" "$ROOT/kernel/src/m054/boot_ui.c" "$ROOT/kernel/src/m048/boot.c" "$ROOT/kernel/src/m030/input.c" "$ROOT/kernel/src/main.c" "$ROOT/kernel/src/m006/cli.c"
# Host test links against tiny stubs for the M015 interface.
cat > /tmp/psre-m054-stubs.c <<'STUB'
#include <psre/m015/xhci_init.h>
static psre_m015_status_t s;
void serial_write(const char *x){(void)x;}
void serial_write_char(char x){(void)x;}
const psre_m015_status_t *psre_m015_get_status(void){return &s;}
bool psre_m015_dma_alloc_pages(uint32_t n,uint64_t*p,void**v){(void)n;(void)p;(void)v;return false;}
bool psre_m015_set_dcbaa_entry(uint8_t i,uint64_t p){(void)i;(void)p;return false;}
STUB
cc -std=c11 -Wall -Wextra -Werror -I "$ROOT/kernel/include" "$ROOT/kernel/src/m054/xhci_slots.c" /tmp/psre-m054-stubs.c "$ROOT/tests/m054/test_slots.c" -o /tmp/psre-m054-test
/tmp/psre-m054-test
echo '[PASS] M054 freestanding integration syntax'
