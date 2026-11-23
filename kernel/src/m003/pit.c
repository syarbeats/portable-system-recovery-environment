#include <stdint.h>
#include <psre/m003/pit.h>
#include <psre/m003/pic.h>
#include <psre/m003/interrupts.h>

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_BASE_HZ 1193182u

static volatile uint64_t g_ticks = 0;

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void pit_irq(psre_interrupt_frame_t *frame) {
    (void)frame;
    ++g_ticks;
}

void psre_pit_initialize(uint32_t frequency_hz) {
    if (frequency_hz == 0) frequency_hz = 100;
    uint32_t divisor = PIT_BASE_HZ / frequency_hz;
    if (divisor == 0) divisor = 1;
    if (divisor > 0xFFFF) divisor = 0xFFFF;

    psre_irq_register(0, pit_irq);

    /* Channel 0, lobyte/hibyte, mode 3, binary. */
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

    psre_pic_clear_mask(0);
}

uint64_t psre_pit_ticks(void) {
    return g_ticks;
}
