#ifndef PSRE_M003_INTERRUPTS_H
#define PSRE_M003_INTERRUPTS_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rdx, rcx, rbx, rax;
    uint64_t vector, error_code;
    uint64_t rip, cs, rflags, rsp, ss;
} psre_interrupt_frame_t;

typedef void (*psre_irq_handler_t)(psre_interrupt_frame_t *);

void psre_idt_initialize(void);
void psre_interrupts_initialize(void);
void psre_interrupt_dispatch(psre_interrupt_frame_t *frame);
void psre_irq_register(uint8_t irq, psre_irq_handler_t handler);
void psre_enable_interrupts(void);
void psre_disable_interrupts(void);

#endif
