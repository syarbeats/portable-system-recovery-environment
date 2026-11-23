#include <psre/m030/input.h>
#include <psre/serial.h>

#define PSRE_M030_INVALID_SUBSCRIPTION 0xFFu

typedef struct {
    bool active;
    uint8_t event_type;
    psre_m030_event_callback_t callback;
    void *context;
} subscriber_t;

static psre_m030_status_t status;
static psre_input_event_t event_queue[PSRE_M030_EVENT_CAPACITY];
static uint16_t queue_head, queue_tail, queue_count;
static subscriber_t subscribers[PSRE_M030_MAX_SUBSCRIBERS];

static void increment_saturated(uint32_t *value) {
    if (*value != UINT32_MAX) ++*value;
}

static int32_t add_saturated_i16(int32_t value, int16_t delta) {
    int64_t result = (int64_t)value + (int64_t)delta;
    if (result > INT32_MAX) return INT32_MAX;
    if (result < INT32_MIN) return INT32_MIN;
    return (int32_t)result;
}

static void update_state(const psre_input_event_t *event) {
    if (event->device == PSRE_INPUT_DEVICE_KEYBOARD) {
        status.modifiers = event->modifiers;
        if (event->type == PSRE_INPUT_KEY_DOWN || event->type == PSRE_INPUT_KEY_REPEAT) {
            status.key_down[event->code >> 3] |= (uint8_t)(1u << (event->code & 7u));
        } else if (event->type == PSRE_INPUT_KEY_UP) {
            status.key_down[event->code >> 3] &= (uint8_t)~(1u << (event->code & 7u));
        }
    } else if (event->device == PSRE_INPUT_DEVICE_MOUSE) {
        if (event->type == PSRE_INPUT_MOUSE_MOVE) {
            status.mouse_x = add_saturated_i16(status.mouse_x, event->delta_x);
            status.mouse_y = add_saturated_i16(status.mouse_y, event->delta_y);
        }
        status.mouse_buttons = event->buttons;
    }
}

static void dispatch(const psre_input_event_t *event) {
    for (uint8_t i = 0; i < PSRE_M030_MAX_SUBSCRIBERS; ++i) {
        subscriber_t *subscriber = &subscribers[i];
        if (subscriber->active &&
            (subscriber->event_type == PSRE_M030_ANY_EVENT ||
             subscriber->event_type == (uint8_t)event->type)) {
            /* Callbacks execute synchronously in the caller's context. */
            subscriber->callback(event, subscriber->context);
            increment_saturated(&status.callback_calls);
        }
    }
}

static void route_event(const psre_input_event_t *event) {
    update_state(event);
    increment_saturated(&status.events_received);
    if (queue_count == PSRE_M030_EVENT_CAPACITY) {
        increment_saturated(&status.events_dropped);
    } else {
        event_queue[queue_tail] = *event;
        queue_tail = (uint16_t)((queue_tail + 1u) % PSRE_M030_EVENT_CAPACITY);
        ++queue_count;
        status.queue_count = queue_count;
        increment_saturated(&status.events_dispatched);
    }
    dispatch(event);
}

bool psre_m030_initialize(void) {
    const psre_m025_status_t *source = psre_m025_get_status();
    if (!source || !source->initialized) {
        serial_write("[M030] M025 must be initialized first.\n");
        return false;
    }
    status = (psre_m030_status_t){0};
    for (uint8_t i = 0; i < PSRE_M030_MAX_SUBSCRIBERS; ++i)
        subscribers[i] = (subscriber_t){0};
    queue_head = queue_tail = queue_count = 0;
    status.initialized = true;
    status.source_available = true;
    serial_write("[M030] Kernel input subsystem initialized.\n");
    return true;
}

bool psre_m030_initialize_headless(void) {
    if (status.initialized) return true;
    status = (psre_m030_status_t){0};
    for (uint8_t i = 0; i < PSRE_M030_MAX_SUBSCRIBERS; ++i)
        subscribers[i] = (subscriber_t){0};
    queue_head = queue_tail = queue_count = 0;
    status.initialized = true;
    status.source_available = false;
    serial_write("[M030] Headless input router initialized; desktop remains usable without HID.\n");
    return true;
}

bool psre_m030_poll(uint32_t timeout_ticks) {
    if (!status.initialized || timeout_ticks == 0u) return false;
    bool source_activity = status.source_available ? psre_m025_poll(timeout_ticks) : false;
    bool routed = false;
    psre_input_event_t event;
    while (psre_m025_next_event(&event)) {
        route_event(&event);
        routed = true;
    }
    return source_activity || routed;
}

bool psre_m030_next_event(psre_input_event_t *event) {
    if (!status.initialized || !event || queue_count == 0u) return false;
    *event = event_queue[queue_head];
    queue_head = (uint16_t)((queue_head + 1u) % PSRE_M030_EVENT_CAPACITY);
    --queue_count;
    status.queue_count = queue_count;
    return true;
}

bool psre_m030_subscribe(uint8_t event_type, psre_m030_event_callback_t callback,
                         void *context, psre_m030_subscription_t *subscription) {
    if (!status.initialized || !callback || !subscription) return false;
    if (event_type != PSRE_M030_ANY_EVENT &&
        (event_type < PSRE_INPUT_KEY_DOWN || event_type > PSRE_INPUT_MOUSE_HWHEEL))
        return false;
    for (uint8_t i = 0; i < PSRE_M030_MAX_SUBSCRIBERS; ++i) {
        if (!subscribers[i].active) {
            subscribers[i].active = true;
            subscribers[i].event_type = event_type;
            subscribers[i].callback = callback;
            subscribers[i].context = context;
            *subscription = i;
            ++status.subscriber_count;
            return true;
        }
    }
    return false;
}

bool psre_m030_unsubscribe(psre_m030_subscription_t subscription) {
    if (!status.initialized || subscription >= PSRE_M030_MAX_SUBSCRIBERS ||
        !subscribers[subscription].active) return false;
    subscribers[subscription] = (subscriber_t){0};
    if (status.subscriber_count) --status.subscriber_count;
    return true;
}

const psre_m030_status_t *psre_m030_get_status(void) { return &status; }

bool psre_m030_key_is_down(uint8_t usage) {
    return status.initialized &&
           (status.key_down[usage >> 3] & (uint8_t)(1u << (usage & 7u))) != 0u;
}

bool psre_m030_mouse_button_is_down(uint8_t button) {
    if (!status.initialized || button < 1u || button > 8u) return false;
    return (status.mouse_buttons & (uint8_t)(1u << (button - 1u))) != 0u;
}

static void print_u32(uint32_t value) {
    char buffer[10]; uint8_t length = 0;
    if (value == 0u) { serial_write("0"); return; }
    while (value && length < sizeof(buffer)) {
        buffer[length++] = (char)('0' + value % 10u);
        value /= 10u;
    }
    while (length) serial_write_char(buffer[--length]);
}

static void print_i32(int32_t value) {
    if (value < 0) {
        serial_write_char('-');
        print_u32((uint32_t)(-(int64_t)value));
    } else {
        print_u32((uint32_t)value);
    }
}

void psre_m030_report(void) {
    serial_write("\n--- M030 KERNEL INPUT SUBSYSTEM ---\nInitialized=");
    serial_write(status.initialized ? "yes" : "no");
    serial_write(" queue="); print_u32(queue_count);
    serial_write(" subscribers="); print_u32(status.subscriber_count);
    serial_write(" received="); print_u32(status.events_received);
    serial_write(" dispatched="); print_u32(status.events_dispatched);
    serial_write(" dropped="); print_u32(status.events_dropped);
    serial_write(" callbacks="); print_u32(status.callback_calls);
    serial_write(" modifiers="); print_u32(status.modifiers);
    serial_write(" mouse_buttons="); print_u32(status.mouse_buttons);
    serial_write(" mouse_x="); print_i32(status.mouse_x);
    serial_write(" mouse_y="); print_i32(status.mouse_y);
    serial_write("\n");
}
