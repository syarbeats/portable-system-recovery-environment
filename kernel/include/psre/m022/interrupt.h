#ifndef PSRE_M022_INTERRUPT_H
#define PSRE_M022_INTERRUPT_H
#include <stdbool.h>
#include <stdint.h>
#define PSRE_M022_MAX_ENDPOINTS 4u
typedef struct {
    bool attempted, endpoint_configured, transfer_pending, transfer_in_flight;
    uint8_t slot_id, endpoint_id, endpoint_address, interval;
    uint16_t max_packet_size;
    uint8_t completion_code;
    uint32_t transfer_count, timeout_count, failure_stage;
    uint32_t transfer_attempts, completion_errors, doorbell_failures;
    uint32_t unmatched_events, event_count, zero_length_transfers;
    uint16_t last_requested, last_actual, last_residual;
    uint8_t last_event_type, last_event_slot, last_event_endpoint;
    uint8_t last_result; /* 0=never, 1=success, 2=timeout, 3=completion error, 4=doorbell, 5=invalid/residual */
} psre_m022_status_t;
/* M022 legacy convenience: configure/read one endpoint. */
bool psre_m022_configure_hid(void);
bool psre_m022_configure_interrupt_in(uint8_t endpoint_address,
                                      uint16_t max_packet_size,
                                      uint8_t interval);
bool psre_m022_interrupt_in(uint8_t *buffer, uint16_t capacity,
                            uint16_t *actual_length, uint32_t timeout_ticks);
/* M026: independently configure and poll up to four Interrupt IN endpoints. */
bool psre_m022_configure_interrupt_in_multi(uint8_t endpoint_address,
                                             uint16_t max_packet_size,
                                             uint8_t interval);
bool psre_m022_interrupt_in_multi(uint8_t endpoint_address, uint8_t *buffer,
                                  uint16_t capacity, uint16_t *actual_length,
                                  uint32_t timeout_ticks);
/* Slot-aware poll (slot 0 = any). Needed when two devices share endpoint address 0x81. */
bool psre_m022_interrupt_in_slot(uint8_t slot_id, uint8_t endpoint_address, uint8_t *buffer,
                                 uint16_t capacity, uint16_t *actual_length, uint32_t timeout_ticks);
const psre_m022_status_t *psre_m022_get_endpoint_status(uint8_t endpoint_address);
void psre_m022_report(void);
const psre_m022_status_t *psre_m022_get_status(void);
#endif
