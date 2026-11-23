#ifndef PSRE_M056_DIAGNOSTICS_H
#define PSRE_M056_DIAGNOSTICS_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {
 bool initialized, framebuffer_ready, usb_controller_ready, device_enumerated;
 bool hid_probe_ok, keyboard_active, mouse_active, desktop_ready, input_router_ready;
 uint8_t hid_interfaces, hid_keyboard_interfaces, hid_mouse_interfaces;
 uint32_t transfer_attempts, transfer_successes, transfer_timeouts;
 uint32_t transfer_completion_errors, transfer_doorbell_errors, transfer_unmatched_events;
 uint32_t keyboard_polls, mouse_polls, input_transfer_errors, routed_events, dropped_events;
} psre_m056_status_t;
bool psre_m056_initialize(void);
void psre_m056_report(void);
const psre_m056_status_t *psre_m056_get_status(void);
#endif
