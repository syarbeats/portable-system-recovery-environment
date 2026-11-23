#ifndef PSRE_M048_BOOT_H
#define PSRE_M048_BOOT_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    bool initialized;
    bool framebuffer_ready;
    bool usb_autostart_requested;
    bool usb_controller_ready;
    bool input_ready;
    bool window_manager_ready;
    bool desktop_ready;
    uint8_t failure_stage;
} psre_m048_status_t;
/* Safe boot integration. USB controller takeover is opt-in at compile time. */
bool psre_m048_initialize(void);
const psre_m048_status_t *psre_m048_get_status(void);
#endif
