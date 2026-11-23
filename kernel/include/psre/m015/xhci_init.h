#ifndef PSRE_M015_XHCI_INIT_H
#define PSRE_M015_XHCI_INIT_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m011/xhci.h>

typedef enum {
    PSRE_M015_STATE_IDLE = 0,
    PSRE_M015_STATE_DISCOVERED,
    PSRE_M015_STATE_OWNERSHIP,
    PSRE_M015_STATE_HALTED,
    PSRE_M015_STATE_RESET,
    PSRE_M015_STATE_CONFIGURED,
    PSRE_M015_STATE_RUNNING,
    PSRE_M015_STATE_FAILED
} psre_m015_state_t;

typedef struct {
    psre_m015_state_t state;
    bool controller_found;
    bool mmio_mapped;
    bool bios_handoff_done;
    bool dma_ready;
    bool busmaster_enabled;
    bool controller_running;
    bool safe_to_free_dma;
    bool dma_64bit_capable;
    uint8_t bus, device, function;
    uint16_t vendor_id, device_id;
    uint64_t bar_address;
    uint8_t max_slots, max_ports;
    uint16_t max_interrupters, scratchpad_count;
    uint32_t cap_length, runtime_offset, doorbell_offset;
    uint32_t timeout_stage;
} psre_m015_status_t;

/* Polled command interface over the rings configured by M015. */
bool psre_m015_submit_command(const psre_xhci_trb_t *command);
bool psre_m015_wait_command_event(psre_xhci_trb_t *event, uint32_t timeout_ticks);
bool psre_m015_wait_event(psre_xhci_trb_t *event, uint32_t timeout_ticks);
bool psre_m015_dma_alloc_pages(uint32_t pages, uint64_t *physical, void **virtual_address);
bool psre_m015_ring_doorbell(uint8_t slot_id, uint8_t target);
uint8_t psre_m015_context_size(void);
bool psre_m015_set_dcbaa_entry(uint8_t slot_id, uint64_t device_context_physical);

/* Explicit hardware operation. Not called automatically during boot. */
bool psre_m015_initialize(void);
void psre_m015_report(void);
const psre_m015_status_t *psre_m015_get_status(void);
#endif
