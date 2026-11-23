#ifndef PSRE_M030_INPUT_H
#define PSRE_M030_INPUT_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m025/input.h>

#define PSRE_M030_EVENT_CAPACITY 128u
#define PSRE_M030_MAX_SUBSCRIBERS 8u
#define PSRE_M030_ANY_EVENT 0u

typedef uint8_t psre_m030_subscription_t;
typedef void (*psre_m030_event_callback_t)(const psre_input_event_t *event, void *context);

typedef struct {
    bool initialized;
    bool source_available; /* false for framebuffer-only desktop boot */
    uint8_t modifiers;
    uint8_t mouse_buttons;
    uint8_t key_down[32]; /* HID Usage ID bitmap, 0..255 */
    int32_t mouse_x, mouse_y;
    uint32_t events_received, events_dispatched, events_dropped;
    uint32_t callback_calls, callback_errors;
    uint16_t queue_count, subscriber_count;
} psre_m030_status_t;

/* Initialize with a live M025 HID source. */
bool psre_m030_initialize(void);
/* Initialize the router without HID so the desktop can render in keyboard/mouse-less mode. */
bool psre_m030_initialize_headless(void);
/* Poll M025 once, route all resulting events, update state and call subscribers. */
bool psre_m030_poll(uint32_t timeout_ticks);
/* Read one event from the M030 independent FIFO. */
bool psre_m030_next_event(psre_input_event_t *event);
/* Subscribe to one event type, or PSRE_M030_ANY_EVENT for all event types. */
bool psre_m030_subscribe(uint8_t event_type, psre_m030_event_callback_t callback,
                         void *context, psre_m030_subscription_t *subscription);
bool psre_m030_unsubscribe(psre_m030_subscription_t subscription);
/* Read a stable snapshot of current input state. */
const psre_m030_status_t *psre_m030_get_status(void);
bool psre_m030_key_is_down(uint8_t hid_usage);
bool psre_m030_mouse_button_is_down(uint8_t button_1_to_8);
void psre_m030_report(void);
#endif
