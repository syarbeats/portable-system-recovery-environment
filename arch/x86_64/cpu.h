#ifndef PSRE_ARCH_X86_64_CPU_H
#define PSRE_ARCH_X86_64_CPU_H

__attribute__((noreturn)) static inline void arch_halt(void) {
    for (;;) {
        __asm__ volatile ("cli; hlt" ::: "memory");
    }
}

static inline void arch_outb(unsigned short port, unsigned char value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline unsigned char arch_inb(unsigned short port) {
    unsigned char value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

#endif
