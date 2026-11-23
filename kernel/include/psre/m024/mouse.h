#ifndef PSRE_M024_MOUSE_H
#define PSRE_M024_MOUSE_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    bool initialized;
    uint8_t interface_number, endpoint_address, interval, buttons; uint16_t max_packet_size;
    int16_t delta_x, delta_y;
    int8_t wheel;
    uint32_t reports, malformed_reports;
} psre_m024_status_t;
/* Supports the 3-byte HID boot mouse report (wheel extension is ignored). */
bool psre_m024_initialize(void);
bool psre_m024_process_report(const uint8_t *report, uint16_t length);
const psre_m024_status_t *psre_m024_get_status(void);
void psre_m024_report(void);
#endif
