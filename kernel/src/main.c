#include <psre/serial.h>
#include <psre/version.h>
#include <psre/memory_map.h>
#include <psre/pmm.h>
#include <psre/heap.h>
#include <psre/vmm.h>
#include <psre/memory_diag.h>
#include <psre/m003/m003.h>
#include <psre/m003/pit.h>
#include <psre/m004/m004.h>
#include <psre/m005/m005.h>
#include <psre/m006/m006.h>
#include <psre/m013/m013.h>
#include <psre/m014/xhci_live.h>
#include <psre/m015/xhci_init.h>
#include <psre/m031/framebuffer.h>
#include <psre/m032/graphics.h>
#include <psre/m035/shell.h>
#include <psre/m048/boot.h>
#include <psre/m049/usb_gui.h>
#include <psre/m050/event_loop.h>
#include <psre/m050/device_manager.h>
#include <psre/m051/device_lifecycle.h>
#include <psre/m052/usb_hub.h>
#include <psre/m053/hub_driver.h>
#include <psre/m054/xhci_slots.h>
#include <psre/m054/boot_ui.h>
#include <psre/m055/input_service.h>
#include <psre/m056/diagnostics.h>
#include <psre/m058/diagnostics.h>
#include <stddef.h>
#include <stdint.h>
#include "../../arch/x86_64/cpu.h"

static void psre_wait_for_pit_test(void) {
    const uint64_t target = 100; /* 1 second at 100 Hz */
    while (psre_pit_ticks() < target) {
        __asm__ volatile("hlt" ::: "memory");
    }
    serial_write("[PASS] PIT delivered at least 100 timer interrupts\n");
}

__attribute__((noreturn))
void psre_kernel_main(void) {
    serial_initialize();
    serial_write("\nPSRE Integrated Build " PSRE_VERSION_STRING " (" PSRE_BUILD_RANGE ") - " PSRE_VERSION_TAG "\n");
    serial_write("Architecture: x86_64 | Boot: Limine/UEFI\n");
    psre_m054_boot_ui_step("Limine kernel entry", "OK");

    /* M002: Limine memory map and HHDM */
    if (!memory_map_initialize()) {
        serial_write("FATAL: Limine memory map/HHDM response unavailable.\n");
        arch_halt();
    }
    serial_write("[PASS] Limine memory map and HHDM acquired\n");
    psre_m054_boot_ui_step("Limine memory map and HHDM", "OK");

    /* M031: discover framebuffer, then start the graphical boot log. */
    if (psre_m031_initialize()) {
        serial_write("[PASS] M031 framebuffer discovered and validated\n");
        psre_m031_report();
        if (psre_m031_draw_test_pattern()) {
            serial_write("[PASS] M031 framebuffer RGB test pattern written\n");
            psre_m054_boot_ui_begin();
            psre_m054_boot_ui_step("Framebuffer pixel write test", "OK");
            psre_m054_boot_ui_step("Framebuffer detected and validated", "OK");
        } else {
            serial_write("[WARN] M031 framebuffer pixel write test failed; graphical boot log disabled\n");
            psre_m054_boot_ui_step("Framebuffer pixel write test", "WARN");
        }
    } else {
        serial_write("[WARN] M031 framebuffer unavailable; serial recovery remains active.\n");
        psre_m031_report();
        psre_m054_boot_ui_step("Framebuffer detection", "WARN");
    }

    if (!pmm_initialize()) {
        serial_write("FATAL: physical page allocator initialization failed.\n");
        arch_halt();
    }
    serial_write("[PASS] Physical page allocator initialized\n");
    psre_m054_boot_ui_step("Physical memory allocator", "OK");

    if (!heap_initialize()) {
        serial_write("FATAL: kernel heap initialization failed.\n");
        arch_halt();
    }
    serial_write("[PASS] Kernel heap initialized\n");
    psre_m054_boot_ui_step("Kernel heap", "OK");

    if (!vmm_initialize()) {
        serial_write("WARN: virtual memory page-table inspection unavailable.\n");
    } else {
        serial_write("[PASS] Current page-table root detected\n");
    }

    uint64_t test_page = pmm_alloc_page();
    if (!test_page || !pmm_free_page(test_page) ||
        pmm_free_page(test_page) || pmm_free_page(test_page + 1)) {
        serial_write("FATAL: PMM allocation/free validation failed.\n");
        arch_halt();
    }
    serial_write("[PASS] PMM allocation/free validation\n");

    size_t heap_before = heap_get_free_bytes();
    void *test_heap = kmalloc(128);
    if (!test_heap) {
        serial_write("FATAL: heap allocation self-test failed.\n");
        arch_halt();
    }
    kfree(test_heap);
    if (heap_get_free_bytes() != heap_before) {
        serial_write("FATAL: heap coalescing self-test failed.\n");
        arch_halt();
    }
    serial_write("[PASS] Heap allocation/free/coalescing\n");

    psre_m054_boot_ui_step("Virtual memory inspection", "OK");
    psre_m054_boot_ui_step("PMM allocation/free self-test", "OK");
    psre_m054_boot_ui_step("Heap allocation/coalescing self-test", "OK");
    memory_diagnostics_run();
    serial_write("[PASS] M002 memory subsystem initialized\n");
    psre_m054_boot_ui_step("Memory diagnostics", "OK");

    /* M003: CPU, GDT, IDT, PIC and PIT. Interrupts enabled at end of setup. */
    psre_m054_boot_ui_step("CPU tables, interrupt handlers and PIT", "INFO");
    psre_m003_init();
    psre_m054_boot_ui_step("CPU / GDT / IDT / PIC / PIT", "OK");
    psre_wait_for_pit_test();
    psre_m054_boot_ui_step("PIT timer interrupt self-test", "OK");

    /* M004: read-only ACPI and PCI hardware inventory. */
    psre_m004_initialize();
    psre_m054_boot_ui_step("ACPI and PCI hardware discovery", "OK");

    /* M013: guarded PCI config writes and MMIO mapping infrastructure. */
    psre_m013_initialize();
    psre_m054_boot_ui_step("PCI configuration and MMIO services", "OK");

    /* M014: read-only live xHCI BAR capability discovery. */
    psre_m014_initialize();
    psre_m054_boot_ui_step("xHCI capability discovery", "OK");

    /* M049: attempt USB HID bring-up before M048 evaluates GUI readiness.
       Makefile defaults M049_AUTOSTART=1 (was 0, which left USB input permanently off).
       Set M049_AUTOSTART=0 to preserve firmware USB ownership. */
    psre_m054_boot_ui_step("USB HID initialization", "INFO");
    bool usb_gui_ready = psre_m049_initialize();
    psre_m054_boot_ui_step("USB HID initialization attempt", usb_gui_ready ? "OK" : "WARN");
    /* Desktop readiness is independent of optional USB HID auto-enumeration. */
    bool desktop_ready = psre_m048_initialize();
    psre_m054_boot_ui_step("PSRE Desktop GUI startup", desktop_ready ? "OK" : "FAIL");
    if (desktop_ready) {
        serial_write("[BOOT] Desktop initialized; USB input status is independent.\n");
    } else {
        serial_write("[BOOT] Desktop unavailable; serial recovery console will be used.\n");
    }
    (void)psre_m055_initialize();
    (void)psre_m056_initialize();
    psre_m058_initialize();
    (void)psre_m050_device_manager_initialize();
    (void)psre_m051_initialize();
    (void)psre_m052_initialize();
    (void)psre_m053_initialize();
    (void)psre_m054_initialize();
    (void)psre_m050_initialize();
    psre_m054_boot_ui_step("Kernel event loop and device services", "OK");

    /* M005: read-only ATA PIO disk discovery and FAT32 BPB probing. */
    psre_m005_initialize();
    psre_m054_boot_ui_step("ATA device discovery", "OK");

    serial_write("[PASS] M005 storage discovery complete\n");
    psre_m054_boot_ui_step("Boot initialization complete", "OK");

    /* Stop boot diagnostics from remaining the final framebuffer painter.
       Repaint the desktop after all boot messages, then keep M050 alive. */
    if (desktop_ready) {
        serial_write("[BOOT] Handing framebuffer to PSRE Desktop.\n");
        (void)psre_m031_enable_backbuffer();   /* flicker-free: draw off-screen, present once per loop */
        (void)psre_m035_render();
        (void)psre_m031_present();
        serial_write("[BOOT] PSRE DESKTOP READY; entering desktop event loop.\n");
        for (;;) {
            psre_m050_run_once();
            (void)psre_m031_present();            /* one copy per loop, only if something was drawn */
            __asm__ volatile("hlt" ::: "memory");
        }
    }

    /* M006 remains the recovery path only when graphical startup failed. */
    psre_m054_boot_ui_step("Recovery console ready (COM1)", "OK");
    psre_m006_initialize();
    for (;;) __asm__ volatile("hlt" ::: "memory");
}
