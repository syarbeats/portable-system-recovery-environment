#ifndef PSRE_M012_XHCI_CAPS_H
#define PSRE_M012_XHCI_CAPS_H
#include <stdbool.h>
#include <stdint.h>

/* Parsed xHCI capability-register snapshot. Input bytes are supplied by a
 * future, correctly mapped MMIO window; this module never dereferences a BAR. */
typedef struct {
    bool valid;
    uint8_t cap_length;
    uint16_t interface_version;
    uint8_t max_slots;
    uint16_t max_interrupters;
    uint8_t max_ports;
    uint32_t doorbell_offset;
    uint32_t runtime_offset;
    uint16_t extended_capability_dword_offset;
} psre_m012_xhci_caps_t;

bool psre_m012_parse_capability_snapshot(const uint8_t *bytes, uint32_t length,
                                         psre_m012_xhci_caps_t *out);
bool psre_m012_software_selftest(void);
void psre_m012_report(void);
#endif
