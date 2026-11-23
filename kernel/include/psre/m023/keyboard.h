#ifndef PSRE_M023_KEYBOARD_H
#define PSRE_M023_KEYBOARD_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    bool initialized;
    uint8_t interface_number, endpoint_address, interval; uint16_t max_packet_size;
    uint8_t modifiers, keys[6];
    uint32_t reports, malformed_reports;
} psre_m023_status_t;
/* Supports HID boot-protocol keyboards (subclass 1, protocol 1). */
bool psre_m023_initialize(void);
bool psre_m023_process_report(const uint8_t *report, uint16_t length);
const psre_m023_status_t *psre_m023_get_status(void);
void psre_m023_report(void);
#endif
