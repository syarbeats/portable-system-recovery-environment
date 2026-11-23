BITS 64
SECTION .text
GLOBAL psre_enable_interrupts
GLOBAL psre_disable_interrupts
psre_enable_interrupts:
    sti
    ret
psre_disable_interrupts:
    cli
    ret
