#ifndef PSRE_M034_WINDOW_H
#define PSRE_M034_WINDOW_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m030/input.h>
#define PSRE_M034_MAX_WINDOWS 8u
#define PSRE_M034_TITLE_CAPACITY 48u
#define PSRE_M034_TEXT_CAPACITY 256u
typedef uint8_t psre_m034_window_id_t;
typedef bool (*psre_m034_desktop_painter_t)(void);
#define PSRE_M034_INVALID_WINDOW 0xFFu
typedef struct {
    bool initialized;
    uint8_t window_count;
    psre_m034_window_id_t focused_window;
    uint32_t events_received, redraws, windows_created, windows_destroyed;
    int32_t cursor_x, cursor_y;
    bool dragging;
} psre_m034_status_t;
typedef struct {
    psre_m034_window_id_t id;
    int32_t x, y, width, height;
    bool visible, focused;
    char title[PSRE_M034_TITLE_CAPACITY];
    char text[PSRE_M034_TEXT_CAPACITY];
} psre_m034_window_info_t;
bool psre_m034_initialize(void);
psre_m034_window_id_t psre_m034_create_window(const char *title, int32_t x, int32_t y,
                                               int32_t width, int32_t height);
bool psre_m034_destroy_window(psre_m034_window_id_t id);
bool psre_m034_set_title(psre_m034_window_id_t id, const char *title);
bool psre_m034_set_position(psre_m034_window_id_t id, int32_t x, int32_t y);
bool psre_m034_set_size(psre_m034_window_id_t id, int32_t width, int32_t height);
bool psre_m034_set_text(psre_m034_window_id_t id, const char *text);
bool psre_m034_append_text(psre_m034_window_id_t id, char ch);
bool psre_m034_focus(psre_m034_window_id_t id);
bool psre_m034_render(void);
/* Optional desktop background painter, called after clear and before windows. */
bool psre_m034_set_desktop_painter(psre_m034_desktop_painter_t painter);
/* Optional overlay painter, called after all windows and before the pointer on EVERY render,
   so taskbar, menus and application content survive redraws caused by input events. */
bool psre_m034_set_overlay_painter(psre_m034_desktop_painter_t painter);
bool psre_m034_handle_event(const psre_input_event_t *event);
bool psre_m034_get_window(psre_m034_window_id_t id, psre_m034_window_info_t *out);
const psre_m034_status_t *psre_m034_get_status(void);
void psre_m034_report(void);
bool psre_m034_draw_demo(void);
#endif
