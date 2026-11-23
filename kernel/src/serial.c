#include <psre/serial.h>
#include "../../arch/x86_64/cpu.h"

#define COM1 0x3F8

void serial_initialize(void) {
    arch_outb(COM1 + 1, 0x00); /* Disable interrupts. */
    arch_outb(COM1 + 3, 0x80); /* Enable divisor latch. */
    arch_outb(COM1 + 0, 0x03); /* 38400 baud, divisor 3. */
    arch_outb(COM1 + 1, 0x00);
    arch_outb(COM1 + 3, 0x03); /* 8 data bits, no parity, one stop bit. */
    arch_outb(COM1 + 2, 0xC7); /* Enable FIFO, clear, 14-byte threshold. */
    arch_outb(COM1 + 4, 0x0B); /* IRQs enabled in modem control; OUT2. */
}

static serial_sink_t g_sink;
void serial_set_sink(serial_sink_t sink) { g_sink = sink; }

void serial_write_char(char c) {
    if (g_sink) g_sink(c);
    /* Bound the wait so a missing UART cannot hang early boot forever. */
    for (unsigned int i = 0; i < 1000000; ++i) {
        if (arch_inb(COM1 + 5) & 0x20) {
            arch_outb(COM1, (unsigned char)c);
            return;
        }
    }
}

void serial_write(const char *text) {
    if (!text) return;
    while (*text) {
        if (*text == '\n') serial_write_char('\r');
        serial_write_char(*text++);
    }
}

bool serial_try_read_char(char *out) {
    if (!out || !(arch_inb(COM1 + 5) & 0x01)) return false;
    *out=(char)arch_inb(COM1); return true;
}

char serial_read_char(void) {
    char value;
    while (!serial_try_read_char(&value)) __asm__ volatile("pause");
    return value;
}

void serial_read_line(char *buffer, unsigned long capacity) {
    if (!buffer || capacity == 0) return;
    unsigned long n = 0;
    for (;;) {
        char c = serial_read_char();
        if (c == '\r' || c == '\n') {
            serial_write("\n");
            break;
        }
        if (c == '\b' || c == 0x7f) {
            if (n) { --n; serial_write("\b \b"); }
            continue;
        }
        if ((unsigned char)c >= 0x20 && (unsigned char)c < 0x7f && n + 1 < capacity) {
            buffer[n++] = c;
            serial_write_char(c);
        }
    }
    buffer[n] = 0;
}
