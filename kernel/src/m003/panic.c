#include <stdint.h>
#include <psre/m003/panic.h>
#include <psre/m003/interrupts.h>

extern void serial_write(const char *message);

static void write_hex(uint64_t value) {
    static const char digits[] = "0123456789ABCDEF";
    char buffer[19] = "0x0000000000000000";
    for (int i = 0; i < 16; ++i) {
        buffer[17 - i] = digits[value & 0xF];
        value >>= 4;
    }
    serial_write(buffer);
}

__attribute__((noreturn))
void psre_kernel_panic(const char *reason) {
    __asm__ volatile("cli");
    serial_write("\n*** PSRE KERNEL PANIC ***\n");
    serial_write(reason ? reason : "unknown");
    serial_write("\nSystem halted.\n");
    for (;;) __asm__ volatile("hlt");
}

__attribute__((noreturn))
void psre_exception_panic(psre_interrupt_frame_t *frame) {
    __asm__ volatile("cli");
    serial_write("\n*** PSRE CPU EXCEPTION ***\nvector=");
    write_hex(frame ? frame->vector : 0xFFFFFFFFu);
    serial_write(" error=");
    write_hex(frame ? frame->error_code : 0);
    serial_write(" rip=");
    write_hex(frame ? frame->rip : 0);
    serial_write("\nSystem halted.\n");
    for (;;) __asm__ volatile("hlt");
}
