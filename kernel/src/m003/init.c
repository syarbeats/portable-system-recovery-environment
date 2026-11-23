#include <stdint.h>
#include <psre/m003/interrupts.h>
#include <psre/m003/cpu.h>
#include <psre/m003/pic.h>
#include <psre/m003/pit.h>
#include <psre/m003/m003.h>

extern void serial_write(const char *message);

static void write_u32(uint32_t value) {
    char buf[11];
    unsigned i = 10;
    buf[i] = '\0';
    do { buf[--i] = (char)('0' + (value % 10)); value /= 10; } while (value && i);
    serial_write(&buf[i]);
}

void psre_cpu_report(void) {
    psre_cpu_info_t info;
    psre_cpu_detect(&info);
    serial_write("[M003] CPU vendor: ");
    serial_write(info.vendor);
    serial_write("\n[M003] CPU brand: ");
    serial_write(info.brand[0] ? info.brand : "not available");
    serial_write("\n[M003] CPU family/model/stepping: ");
    write_u32(info.family); serial_write("/");
    write_u32(info.model); serial_write("/");
    write_u32(info.stepping);
    serial_write("\n");
}

void psre_interrupts_initialize(void) {
    psre_disable_interrupts();
    psre_idt_initialize();
    serial_write("[M003] IDT installed\n");
    psre_pic_initialize();
    serial_write("[M003] PIC remapped: master=0x20 slave=0x28\n");
    psre_pit_initialize(100);
    serial_write("[M003] PIT configured: 100 Hz\n");
}

void psre_m003_init(void) {
    psre_disable_interrupts();
    psre_cpu_report();
    psre_gdt_initialize();
    serial_write("[M003] GDT loaded (kernel code=0x08 data=0x10)\n");
    psre_interrupts_initialize();
    psre_enable_interrupts();
    serial_write("[M003] interrupts enabled\n");
}
