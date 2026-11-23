#ifndef PSRE_M028_MOUSE_H
#define PSRE_M028_MOUSE_H
#include <stdbool.h>
#include <stdint.h>
#define PSRE_M028_MAX_FIELDS 32u
#define PSRE_M028_MAX_REPORT_IDS 16u
#define PSRE_M028_MAX_REPORT_DESCRIPTOR 512u

typedef struct {
    bool initialized, report_protocol, has_report_ids;
    uint8_t interface_number, endpoint_address, interval;
    uint16_t max_packet_size, descriptor_length;
    uint8_t buttons;
    int16_t delta_x, delta_y;
    int8_t wheel, horizontal_wheel;
    uint32_t reports, malformed_reports, descriptor_errors;
    uint32_t unsupported_fields;
} psre_m028_status_t;

/* HID Report Protocol mouse parser: X/Y, wheel, horizontal pan and buttons 1..8. */
bool psre_m028_initialize(void);
bool psre_m028_process_report(const uint8_t *report, uint16_t length);
const psre_m028_status_t *psre_m028_get_status(void);
void psre_m028_report(void);
#endif
