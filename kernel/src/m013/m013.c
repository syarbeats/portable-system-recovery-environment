#include <psre/m013/m013.h>
#include <psre/vmm.h>
#include <psre/m004/pci.h>
#include <psre/serial.h>
#include <stdint.h>

static bool selftest_passed;

bool psre_m013_selftest(void) {
    bool ok=true;
    if(vmm_map_mmio(0,4096)!=0)ok=false;
    if(vmm_map_mmio(0xFEC00000ULL,0)!=0)ok=false;
    if(vmm_unmap_mmio(0,4096))ok=false;
    selftest_passed=ok;
    serial_write(ok ? "[M013] MMIO guard self-test PASS (invalid inputs rejected).\n" :
                      "[M013] MMIO guard self-test FAIL.\n");
    return ok;
}

void psre_m013_initialize(void) {
    serial_write("\n=== PSRE M013 PCI WRITE & MMIO MAPPING ===\n");
    (void)psre_m013_selftest();
    psre_m013_report();
}

void psre_m013_report(void) {
    const psre_pci_stats_t *pci=psre_pci_get_stats();
    serial_write("--- PSRE M013 STATUS ---\n");
    serial_write(pci && pci->config_mechanism_1_available ?
                 "[M013] PCI config mechanism #1: AVAILABLE\n" :
                 "[M013] PCI config mechanism #1: UNAVAILABLE\n");
    serial_write("[M013] PCI config read/write API: READY (explicit calls only)\n");
    serial_write("[M013] MMIO page mapper: READY (4 KiB, UC, kernel-only)\n");
    serial_write(selftest_passed ? "[M013] Guard self-test: PASS\n" :
                                  "[M013] Guard self-test: NOT RUN / FAIL\n");
    serial_write("[M013] Active MMIO mappings: ");
    /* Small decimal conversion without libc. */
    uint32_t n=vmm_mmio_active_mappings();
    char digits[11];unsigned count=0;
    if(!n)digits[count++]='0';
    while(n && count<sizeof(digits)){digits[count++]=(char)('0'+n%10);n/=10;}
    while(count)serial_write_char(digits[--count]);
    serial_write("\n[M013] Bus-master enable is explicit and is NOT called automatically.\n");
    serial_write("[M013] No live device BAR is mapped or accessed during initialization.\n");
}
