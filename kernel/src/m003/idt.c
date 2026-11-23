#include <stdint.h>
#include <stddef.h>
#include <psre/m003/interrupts.h>
#include <psre/m003/panic.h>
#include <psre/m003/pic.h>

extern void *psre_isr_stub_table[48];

typedef struct __attribute__((packed)) {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} idt_entry_t;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
} idtr_t;

static idt_entry_t g_idt[256];
static psre_irq_handler_t g_irq_handlers[16];

static void idt_set_gate(uint8_t vector, void *handler, uint8_t flags) {
    uint64_t address = (uint64_t)handler;
    g_idt[vector].offset_low = (uint16_t)(address & 0xFFFF);
    g_idt[vector].selector = 0x08;
    g_idt[vector].ist = 0;
    g_idt[vector].type_attr = flags;
    g_idt[vector].offset_mid = (uint16_t)((address >> 16) & 0xFFFF);
    g_idt[vector].offset_high = (uint32_t)(address >> 32);
    g_idt[vector].reserved = 0;
}

static inline void lidt(const idtr_t *idtr) {
    __asm__ volatile("lidt %0" : : "m"(*idtr) : "memory");
}

void psre_irq_register(uint8_t irq, psre_irq_handler_t handler) {
    if (irq < 16) g_irq_handlers[irq] = handler;
}

void psre_idt_initialize(void) {
    for (uint32_t i = 0; i < 256; ++i) {
        g_idt[i] = (idt_entry_t){0};
    }

    for (uint32_t i = 0; i < 48; ++i) {
        /* 0x8E = present, ring 0, 64-bit interrupt gate */
        idt_set_gate((uint8_t)i, psre_isr_stub_table[i], 0x8E);
    }

    idtr_t idtr = {
        .limit = (uint16_t)(sizeof(g_idt) - 1),
        .base = (uint64_t)&g_idt[0]
    };
    lidt(&idtr);
}

void psre_interrupt_dispatch(psre_interrupt_frame_t *frame) {
    if (!frame) psre_kernel_panic("null interrupt frame");

    uint64_t vector = frame->vector;

    if (vector < 32) {
        psre_exception_panic(frame);
    }

    if (vector >= 32 && vector <= 47) {
        uint8_t irq = (uint8_t)(vector - 32);
        if (g_irq_handlers[irq]) g_irq_handlers[irq](frame);
        psre_pic_send_eoi(irq);
        return;
    }

    /* Unexpected vectors are logged as fatal in this baseline. */
    psre_kernel_panic("unexpected interrupt vector");
}
