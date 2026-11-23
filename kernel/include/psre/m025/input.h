#ifndef PSRE_M025_INPUT_H
#define PSRE_M025_INPUT_H
#include <stdbool.h>
#include <stdint.h>
#define PSRE_M025_EVENT_CAPACITY 64u
typedef enum { PSRE_INPUT_KEY_DOWN=1, PSRE_INPUT_KEY_UP, PSRE_INPUT_MOUSE_MOVE, PSRE_INPUT_MOUSE_BUTTON_DOWN, PSRE_INPUT_MOUSE_BUTTON_UP, PSRE_INPUT_KEY_REPEAT, PSRE_INPUT_MOUSE_WHEEL, PSRE_INPUT_MOUSE_HWHEEL } psre_input_event_type_t;
typedef enum { PSRE_INPUT_DEVICE_NONE=0, PSRE_INPUT_DEVICE_KEYBOARD, PSRE_INPUT_DEVICE_MOUSE } psre_input_device_t;
typedef struct { psre_input_event_type_t type; uint8_t device, code, modifiers, buttons; int16_t delta_x, delta_y; int8_t wheel, horizontal_wheel; char character; } psre_input_event_t;
typedef struct { bool initialized, endpoint_configured, keyboard_active, mouse_active; psre_input_device_t active_device; uint8_t interface_number, endpoint_address; uint32_t polls, keyboard_polls, mouse_polls, transfer_errors, events_generated, events_dropped; uint16_t queue_count; } psre_m025_status_t;
bool psre_m025_initialize(void);
/* Add the HID device currently enumerated (call once per device; keyboard and mouse may be separate). */
bool psre_m025_add_device(void);
/* Polls every active endpoint once; timeout is applied separately per endpoint. */
bool psre_m025_poll(uint32_t timeout_ticks);
bool psre_m025_next_event(psre_input_event_t *event);
const psre_m025_status_t *psre_m025_get_status(void);
void psre_m025_report(void);
#endif
