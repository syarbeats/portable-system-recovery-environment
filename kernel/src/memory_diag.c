#include <psre/memory_diag.h>
#include <psre/serial.h>
#include <psre/memory_map.h>
#include <psre/pmm.h>
#include <psre/heap.h>
#include <psre/vmm.h>

static void write_u64(uint64_t value) {
    char buf[21]; unsigned int i = 0;
    if (value == 0) { serial_write("0"); return; }
    while (value && i < sizeof(buf)) { buf[i++] = (char)('0' + value % 10); value /= 10; }
    while (i) serial_write_char(buf[--i]);
}
static void write_hex(uint64_t value) {
    static const char digits[] = "0123456789ABCDEF";
    char buf[16]; unsigned int i = 0;
    serial_write("0x");
    if (!value) { serial_write("0"); return; }
    while (value && i < sizeof(buf)) { buf[i++] = digits[value & 0xf]; value >>= 4; }
    while (i) serial_write_char(buf[--i]);
}

void memory_diagnostics_run(void) {
    const struct psre_memory_map_stats *mm = memory_map_get_stats();
    const struct psre_pmm_stats *pm = pmm_get_stats();
    serial_write("\n--- PSRE MEMORY DIAGNOSTICS ---\n");
    serial_write("Memory map entries: "); write_u64(mm->entry_count); serial_write("\n");
    serial_write("Usable RAM bytes:   "); write_u64(mm->usable_bytes); serial_write("\n");
    serial_write("Highest address:    "); write_hex(mm->highest_address); serial_write("\n");
    serial_write("PMM managed limit:  "); write_u64(pm->managed_limit_bytes); serial_write(" bytes\n");
    serial_write("Free pages:         "); write_u64(pm->free_pages); serial_write("\n");
    serial_write("Allocated pages:    "); write_u64(pm->allocated_pages); serial_write("\n");
    serial_write("Free RAM (PMM):     "); write_u64(pm->free_pages * PSRE_PAGE_SIZE); serial_write(" bytes\n");
    serial_write("Heap capacity:      "); write_u64(heap_get_capacity()); serial_write(" bytes\n");
    serial_write("Heap free:          "); write_u64(heap_get_free_bytes()); serial_write(" bytes\n");
    serial_write("CR3:                "); write_hex(vmm_get_cr3()); serial_write("\n");
    serial_write("HHDM offset:        "); write_hex(memory_map_hhdm_offset()); serial_write("\n");
    serial_write("--------------------------------\n");
}
