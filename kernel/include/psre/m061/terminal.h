#ifndef PSRE_M061_TERMINAL_H
#define PSRE_M061_TERMINAL_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m030/input.h>
#include <psre/m036/app.h>

/* M061 GUI Terminal: a windowed command prompt that runs every command of the PSRE
   recovery shell (psre_m006_execute) and shows its output in the window. */
#define PSRE_M061_MAX_COLS 120u
#define PSRE_M061_MAX_LINES 256u
#define PSRE_M061_INPUT_MAX 160u
#define PSRE_M061_HISTORY 16u

/* Scroll bar geometry (framebuffer coordinates) - used for drawing, hit-testing and tests. */
typedef struct {
    int32_t x, y, w, h;              /* whole bar column */
    int32_t up_y, down_y;            /* top of the arrow buttons (w x w) */
    int32_t track_y, track_h;        /* area between the arrows */
    int32_t thumb_y, thumb_h;
    uint32_t total, rows, scroll, max_scroll;
} psre_m061_scrollbar_t;

typedef struct {
    bool open;
    uint32_t commands_run, commands_blocked, lines_written, renders;
    uint16_t cols, rows;
    uint32_t scroll, max_scroll;
} psre_m061_status_t;

/* Prepare the terminal for the given (already launched) M036 application. */
bool psre_m061_open(psre_m036_app_id_t app);
/* M036 event handler: keyboard editing, history, scrolling. */
bool psre_m061_on_event(psre_m036_app_id_t app, const psre_input_event_t *event, void *state);
/* Draw the terminal inside its window (called from the desktop overlay painter). */
bool psre_m061_render(void);
/* Current scroll bar layout; false while the terminal is closed or not drawable. */
bool psre_m061_get_scrollbar(psre_m061_scrollbar_t *out);
/* Run one line exactly as if typed + Enter (echo included). */
void psre_m061_run_line(const char *line);
/* back=0 is the newest finished line; copies it as text. */
bool psre_m061_copy_line(uint32_t back, char *out, uint32_t capacity);
const psre_m061_status_t *psre_m061_get_status(void);
void psre_m061_report(void);
#endif
