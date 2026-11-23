#ifndef PSRE_M055_INPUT_SERVICE_H
#define PSRE_M055_INPUT_SERVICE_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    bool initialized;
    bool usb_input_available;
    bool keyboard_active;
    bool mouse_active;
    uint64_t poll_cycles;
    uint64_t active_poll_cycles;
    uint64_t idle_cycles;
    uint64_t poll_errors;
} psre_m055_status_t;
/* Initializes the input service without making GUI startup depend on USB. */
bool psre_m055_initialize(void);
/* Bounded HID/event pump. Safe to call when no USB device is available. */
bool psre_m055_poll(uint32_t timeout_ticks);
const psre_m055_status_t *psre_m055_get_status(void);
void psre_m055_report(void);
#endif
