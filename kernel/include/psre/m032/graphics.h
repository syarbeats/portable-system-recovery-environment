#ifndef PSRE_M032_GRAPHICS_H
#define PSRE_M032_GRAPHICS_H
#include <stdbool.h>
#include <stdint.h>

typedef struct { uint8_t red, green, blue; } psre_m032_color_t;
typedef enum {
    PSRE_M032_BITMAP_RGB24 = 0,
    PSRE_M032_BITMAP_BGR24 = 1
} psre_m032_bitmap_format_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    const uint8_t *pixels;
    psre_m032_bitmap_format_t format;
} psre_m032_bitmap_t;

typedef struct { int32_t x, y, width, height; } psre_m032_rect_t;

bool psre_m032_initialize(void);
bool psre_m032_clear(psre_m032_color_t color);
bool psre_m032_draw_pixel(int32_t x, int32_t y, psre_m032_color_t color);
bool psre_m032_draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, psre_m032_color_t color);
bool psre_m032_draw_rect(int32_t x, int32_t y, int32_t width, int32_t height, psre_m032_color_t color);
bool psre_m032_fill_rect(int32_t x, int32_t y, int32_t width, int32_t height, psre_m032_color_t color);
bool psre_m032_draw_circle(int32_t cx, int32_t cy, int32_t radius, psre_m032_color_t color);
bool psre_m032_fill_circle(int32_t cx, int32_t cy, int32_t radius, psre_m032_color_t color);
bool psre_m032_draw_triangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
                             int32_t x2, int32_t y2, psre_m032_color_t color);
bool psre_m032_fill_triangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
                             int32_t x2, int32_t y2, psre_m032_color_t color);
bool psre_m032_draw_bitmap(int32_t x, int32_t y, const psre_m032_bitmap_t *bitmap);
bool psre_m032_clip_rect(psre_m032_rect_t *rect);
void psre_m032_report(void);
bool psre_m032_draw_demo(void);
#endif
