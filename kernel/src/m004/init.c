#include <psre/m004/m004.h>
#include <psre/m004/pci.h>
#include <psre/m004/acpi.h>
#include <psre/m010/usb.h>
#include <psre/m011/xhci.h>
#include <psre/m012/xhci_caps.h>
#include <psre/serial.h>
void psre_m004_initialize(void) {
    serial_write("\n=== PSRE M004 HARDWARE DETECTION ===\n");
    if (psre_acpi_initialize()) serial_write("[PASS] ACPI RSDP/root table parsed\n");
    else serial_write("[WARN] ACPI RSDP/root table unavailable or invalid\n");
    psre_acpi_report();
    if (psre_pci_initialize()) {
        serial_write("[PASS] PCI config mechanism #1 initialized\n");
        psre_pci_report();
        if (psre_m010_usb_initialize()) psre_m010_usb_report();
        else serial_write("[M010] USB controller discovery skipped: PCI unavailable\n");
        if (psre_m011_initialize()) {
            psre_m011_report();
            (void)psre_m011_software_selftest();
        } else {
            psre_m011_report();
        }
        /* M012 parser tests are hardware-independent. */
        (void)psre_m012_software_selftest();
        psre_m012_report();
    } else {
        serial_write("[WARN] PCI enumeration unavailable\n");
    }
    serial_write("[M004] Hardware inventory collection complete\n");
}
