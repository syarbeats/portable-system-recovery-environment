#include <stdint.h>

/*
 * Minimal long-mode GDT reload.
 * Selectors: 0x08 kernel code, 0x10 kernel data.
 * A TSS descriptor and IST stacks are intentionally deferred to hardening work.
 */
extern void psre_gdt_load(const void *gdtr);

struct __attribute__((packed)) gdt_ptr {
    uint16_t limit;
    uint64_t base;
};

static uint64_t gdt[3] __attribute__((aligned(16))) = {
    0x0000000000000000ULL,
    0x00AF9A000000FFFFULL, /* 64-bit ring-0 code */
    0x00CF92000000FFFFULL  /* ring-0 data */
};

void psre_gdt_initialize(void) {
    struct gdt_ptr ptr = {
        .limit = (uint16_t)(sizeof(gdt) - 1),
        .base = (uint64_t)&gdt[0]
    };
    psre_gdt_load(&ptr);
}
