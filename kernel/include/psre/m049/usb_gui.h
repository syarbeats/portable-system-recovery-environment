#ifndef PSRE_M049_USB_GUI_H
#define PSRE_M049_USB_GUI_H
#include <stdbool.h>
#include <stdint.h>
typedef enum {
    PSRE_M049_STAGE_IDLE=0, PSRE_M049_STAGE_XHCI, PSRE_M049_STAGE_PORT,
    PSRE_M049_STAGE_SLOT, PSRE_M049_STAGE_ADDRESS, PSRE_M049_STAGE_CONFIG,
    PSRE_M049_STAGE_HID, PSRE_M049_STAGE_INPUT, PSRE_M049_STAGE_GUI,
    PSRE_M049_STAGE_READY, PSRE_M049_STAGE_FAILED
} psre_m049_stage_t;
typedef struct {
    bool attempted, initialized, controller_ready, device_enumerated;
    bool configuration_ready, hid_ready, input_ready, gui_ready;
    bool keyboard_ready, mouse_ready;
    uint8_t selected_port, ports_examined;
    psre_m049_stage_t stage;
    uint32_t enumeration_failures, event_polls, events_dispatched;
} psre_m049_status_t;
/* Compile with -DPSRE_M049_ENABLE_AUTOSTART=1 to enable xHCI takeover and
   one-device USB HID enumeration during boot. Default is safe/off. */
bool psre_m049_initialize(void);
/* Bounded, synchronous input/event pump. Call regularly from the kernel loop. */
bool psre_m049_poll(uint32_t timeout_ticks);
const psre_m049_status_t *psre_m049_get_status(void);
#endif
