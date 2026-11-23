#ifndef PSRE_M035_SHELL_H
#define PSRE_M035_SHELL_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m034/window.h>

typedef enum {
    PSRE_M035_APP_RECOVERY = 0,
    PSRE_M035_APP_SYSTEM_INFO,
    PSRE_M035_APP_USB_MANAGER,
    PSRE_M035_APP_SETTINGS,
    PSRE_M035_APP_FILE_EXPLORER,
    PSRE_M035_APP_TERMINAL,
    PSRE_M035_APP_COUNT
} psre_m035_app_t;

typedef struct {
    bool initialized;
    bool start_menu_open;
    uint8_t selected_menu_item;
    uint32_t input_events, redraws, app_launches;
    uint64_t uptime_seconds;
} psre_m035_status_t;

bool psre_m035_initialize(void);
bool psre_m035_render(void);
bool psre_m035_draw_desktop(void);
bool psre_m035_handle_event(const psre_input_event_t *event);
bool psre_m035_launch_app(psre_m035_app_t app);
void psre_m035_toggle_start_menu(void);
const psre_m035_status_t *psre_m035_get_status(void);
void psre_m035_report(void);
#endif
