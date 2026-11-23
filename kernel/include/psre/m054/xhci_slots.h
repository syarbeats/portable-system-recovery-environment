#ifndef PSRE_M054_XHCI_SLOTS_H
#define PSRE_M054_XHCI_SLOTS_H
#include <stdbool.h>
#include <stdint.h>
#define PSRE_M054_MAX_SLOTS 32u
typedef enum { PSRE_M054_SLOT_EMPTY=0, PSRE_M054_SLOT_ENABLED, PSRE_M054_SLOT_CONTEXT_READY, PSRE_M054_SLOT_ADDRESSED, PSRE_M054_SLOT_CONFIGURED, PSRE_M054_SLOT_ACTIVE, PSRE_M054_SLOT_REMOVING, PSRE_M054_SLOT_ERROR } psre_m054_slot_state_t;
typedef struct {
 bool occupied; uint8_t slot_id, root_port, speed_id; psre_m054_slot_state_t state;
 uint16_t vendor_id, product_id; uint64_t device_context_phys; void *device_context_virt;
 uint64_t ep0_ring_phys; void *ep0_ring_virt; uint32_t generation;
} psre_m054_slot_t;
typedef struct { bool initialized; uint8_t capacity, active_slots; uint32_t allocations, rejected, state_changes; } psre_m054_status_t;
bool psre_m054_initialize(void);
bool psre_m054_register_slot(uint8_t slot_id, uint8_t root_port, uint8_t speed_id);
bool psre_m054_set_identity(uint8_t slot_id, uint16_t vendor_id, uint16_t product_id);
bool psre_m054_set_state(uint8_t slot_id, psre_m054_slot_state_t state);
bool psre_m054_unregister_slot(uint8_t slot_id);
const psre_m054_slot_t *psre_m054_get_slot(uint8_t slot_id);
const psre_m054_status_t *psre_m054_get_status(void);
void psre_m054_report(void);
#endif
