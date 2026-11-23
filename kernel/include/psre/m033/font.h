#ifndef PSRE_M033_FONT_H
#define PSRE_M033_FONT_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m032/graphics.h>

typedef enum {
    PSRE_M033_FONT_8X8 = 0,
    PSRE_M033_FONT_8X16 = 1
} psre_m033_font_t;

typedef struct {
    int32_t x, y;
    uint32_t width, height;
    uint8_t spacing;
    psre_m033_font_t font;
    psre_m032_color_t foreground;
    psre_m032_color_t background;
    bool opaque_background;
} psre_m033_text_style_t;

typedef struct { uint32_t width, height; } psre_m033_text_size_t;

bool psre_m033_initialize(void);
/* UI text size. In "large" mode every PSRE_M033_FONT_8X8 request is drawn with the taller
   8x16 glyph set cropped to 13 rows (cap height ~9 px instead of ~6 px). The character advance
   is unchanged (8 + spacing), so horizontal layouts stay valid; the line pitch becomes 14 px. */
void psre_m033_set_ui_large(bool large);
bool psre_m033_ui_large(void);
/* Line pitch for PSRE_M033_FONT_8X8 text in the current mode (8 classic, 14 large). */
uint8_t psre_m033_ui_line_height(void);
bool psre_m033_draw_char(int32_t x, int32_t y, char ch, psre_m033_font_t font,
                         psre_m032_color_t foreground, psre_m032_color_t background,
                         bool opaque_background);
bool psre_m033_draw_string(const char *text, const psre_m033_text_style_t *style);
psre_m033_text_size_t psre_m033_measure(const char *text, psre_m033_font_t font, uint8_t spacing);
void psre_m033_report(void);
bool psre_m033_draw_demo(void);
#endif
