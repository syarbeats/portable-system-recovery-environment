#ifndef PSRE_M045_BULK_H
#define PSRE_M045_BULK_H
#include <stdbool.h>
#include <stdint.h>
#define PSRE_M045_MAX_ENDPOINTS 4u
#define PSRE_M045_MAX_TRANSFER 4096u
typedef struct { bool configured, pending, in; uint8_t address, endpoint_id, slot_id, completion_code; uint16_t max_packet; uint32_t transfers, timeouts, errors; } psre_m045_endpoint_status_t;
typedef struct { uint32_t configured_endpoints, transfers, timeouts, errors, last_residual; } psre_m045_status_t;
bool psre_m045_configure_bulk_endpoint(uint8_t address, uint16_t max_packet);
bool psre_m045_bulk_transfer(uint8_t address, void *buffer, uint32_t length, uint32_t timeout_ticks, uint32_t *actual_length);
const psre_m045_endpoint_status_t *psre_m045_get_endpoint_status(uint8_t address);
const psre_m045_status_t *psre_m045_get_status(void);
void psre_m045_report(void);
#endif
