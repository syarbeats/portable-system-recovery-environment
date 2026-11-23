#ifndef PSRE_M014_XHCI_LIVE_H
#define PSRE_M014_XHCI_LIVE_H
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool controller_found;
    bool bar_valid;
    bool mmio_mapped;
    bool capability_valid;
    uint8_t bus, device, function, bar_index;
    uint16_t vendor_id, device_id, interface_version;
    uint64_t bar_address;
    uint8_t cap_length, max_slots, max_ports;
    uint16_t max_interrupters, xecp_dword_offset;
    uint32_t doorbell_offset, runtime_offset;
    uint32_t mmio_active_after_probe;
} psre_m014_xhci_live_status_t;

/* Read-only live probe. No PCI command-register or controller-register writes. */
bool psre_m014_probe_xhci(void);
void psre_m014_initialize(void);
void psre_m014_report(void);
const psre_m014_xhci_live_status_t *psre_m014_get_status(void);
#endif
