#ifndef PSRE_M036_APP_H
#define PSRE_M036_APP_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m034/window.h>
#include <psre/m032/graphics.h>
#define PSRE_M036_MAX_APPS 8u
#define PSRE_M036_STATE_CAPACITY 64u
#define PSRE_M036_INVALID_APP 0xFFu

typedef uint8_t psre_m036_app_id_t;
typedef enum {
    PSRE_M036_REGISTERED = 0,
    PSRE_M036_RUNNING,
    PSRE_M036_STOPPED,
    PSRE_M036_ERROR
} psre_m036_lifecycle_t;
typedef bool (*psre_m036_event_handler_t)(psre_m036_app_id_t app,
                                           const psre_input_event_t *event,
                                           void *state);
typedef struct {
    const char *title;
    const char *initial_text;
    psre_m036_event_handler_t on_event;
} psre_m036_app_descriptor_t;
typedef struct {
    bool registered;
    psre_m036_lifecycle_t lifecycle;
    psre_m034_window_id_t window;
    uint32_t launches, events;
    uint8_t state[PSRE_M036_STATE_CAPACITY];
    char title[PSRE_M034_TITLE_CAPACITY];
} psre_m036_app_info_t;
typedef struct {
    bool initialized;
    uint8_t registered_apps, running_apps;
    uint32_t launches, stops, dispatched_events, dispatch_misses, errors;
} psre_m036_status_t;

bool psre_m036_initialize(void);
psre_m036_app_id_t psre_m036_register(const psre_m036_app_descriptor_t *descriptor);
bool psre_m036_launch(psre_m036_app_id_t app, int32_t x, int32_t y,
                       int32_t width, int32_t height);
bool psre_m036_stop(psre_m036_app_id_t app);
bool psre_m036_dispatch_event(const psre_input_event_t *event);
bool psre_m036_reconcile(void);
bool psre_m036_get_app(psre_m036_app_id_t app, psre_m036_app_info_t *out);
void *psre_m036_get_state(psre_m036_app_id_t app);
const psre_m036_status_t *psre_m036_get_status(void);
void psre_m036_report(void);

/* M036 standard drawing controls; coordinates are framebuffer-relative. */
bool psre_m036_label(const char *caption, int32_t x, int32_t y, uint32_t width,
                      psre_m032_color_t foreground);
bool psre_m036_button(const char *caption, int32_t x, int32_t y, int32_t width,
                       int32_t height, bool pressed);
bool psre_m036_textbox(const char *value, int32_t x, int32_t y, int32_t width,
                        int32_t height, bool focused);
bool psre_m036_checkbox(const char *caption, int32_t x, int32_t y, bool checked);
#endif
