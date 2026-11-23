#ifndef PSRE_M011_XHCI_H
#define PSRE_M011_XHCI_H
#include <stdbool.h>
#include <stdint.h>
typedef struct __attribute__((packed, aligned(16))) {
    uint64_t parameter;
    uint32_t status;
    uint32_t control;
} psre_xhci_trb_t;
typedef struct {
    uint64_t segment_base;
    uint32_t segment_size;
    uint32_t reserved;
} __attribute__((packed)) psre_xhci_erst_entry_t;
typedef struct {
    bool controller_found;
    bool dma_resources_ready;
    uint8_t bus, device, function;
    uint16_t vendor_id, device_id;
    uint64_t bar_address;
    uint32_t dma_pages;
    uint32_t command_enqueue;
    uint32_t event_dequeue;
} psre_m011_xhci_status_t;
bool psre_m011_initialize(void);
const psre_m011_xhci_status_t *psre_m011_get_status(void);
void psre_m011_report(void);
bool psre_m011_software_selftest(void);
void psre_m011_release(void);
#endif
