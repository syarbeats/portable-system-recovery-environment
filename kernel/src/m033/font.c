#include <psre/m033/font.h>
#include <psre/m031/framebuffer.h>
#include <psre/serial.h>
#include "font_data.h"

static bool g_initialized;
static uint64_t g_chars_drawn;
static bool g_ui_large;
#define LARGE_ROW0 2u      /* all 8x16 glyphs use rows 2..14 only */
#define LARGE_ROWS 13u
#define LARGE_PITCH 14u
static bool is_large(psre_m033_font_t f) { return g_ui_large && f == PSRE_M033_FONT_8X8; }
static uint8_t font_height(psre_m033_font_t f) {
    if (is_large(f)) return LARGE_PITCH;
    return f == PSRE_M033_FONT_8X8 ? 8u : 16u;
}
void psre_m033_set_ui_large(bool large) { g_ui_large = large; }
bool psre_m033_ui_large(void) { return g_ui_large; }
uint8_t psre_m033_ui_line_height(void) { return font_height(PSRE_M033_FONT_8X8); }
static bool valid_font(psre_m033_font_t f) { return f == PSRE_M033_FONT_8X8 || f == PSRE_M033_FONT_8X16; }

bool psre_m033_initialize(void) {
    g_initialized = psre_m032_initialize();
    g_chars_drawn = 0;
    return g_initialized;
}

static bool draw_glyph(int32_t x, int32_t y, char ch, psre_m033_font_t font,
                       psre_m032_color_t fg, psre_m032_color_t bg, bool opaque,
                       int64_t clip_x0, int64_t clip_y0, int64_t clip_x1, int64_t clip_y1) {
    if (!g_initialized || !valid_font(font)) return false;
    const psre_m031_framebuffer_t *fb = psre_m031_get();
    if (!fb->available) return false;
    if (clip_x0 < 0) clip_x0 = 0;
    if (clip_y0 < 0) clip_y0 = 0;
    if (clip_x1 > (int64_t)fb->width) clip_x1 = (int64_t)fb->width;
    if (clip_y1 > (int64_t)fb->height) clip_y1 = (int64_t)fb->height;
    if (clip_x0 >= clip_x1 || clip_y0 >= clip_y1) return true;
    uint8_t glyph_height = is_large(font) ? (uint8_t)LARGE_ROWS : font_height(font);
    const uint8_t *glyph;
    if ((uint8_t)ch < 32u || (uint8_t)ch > 126u) ch = '?';
    uint8_t index = (uint8_t)ch - 32u;
    if (is_large(font)) glyph = &psre_font_8x16[index][LARGE_ROW0];
    else glyph = font == PSRE_M033_FONT_8X8 ? psre_font_8x8[index] : psre_font_8x16[index];

    bool touched = false;
    for (uint8_t row=0; row<glyph_height; ++row) {
        int64_t py = (int64_t)y + row;
        if (py < clip_y0 || py >= clip_y1) continue;
        for (uint8_t col=0; col<8u; ++col) {
            int64_t px = (int64_t)x + col;
            if (px < clip_x0 || px >= clip_x1) continue;
            bool set = (glyph[row] & (uint8_t)(0x80u >> col)) != 0;
            if (set) touched = psre_m032_draw_pixel((int32_t)px, (int32_t)py, fg) || touched;
            else if (opaque) touched = psre_m032_draw_pixel((int32_t)px, (int32_t)py, bg) || touched;
        }
    }
    if (touched) ++g_chars_drawn;
    return true;
}

bool psre_m033_draw_char(int32_t x, int32_t y, char ch, psre_m033_font_t font,
                         psre_m032_color_t fg, psre_m032_color_t bg, bool opaque) {
    const psre_m031_framebuffer_t *fb = psre_m031_get();
    if (!fb->available) return false;
    return draw_glyph(x,y,ch,font,fg,bg,opaque,0,0,(int64_t)fb->width,(int64_t)fb->height);
}

bool psre_m033_draw_string(const char *text, const psre_m033_text_style_t *style) {
    if (!g_initialized || !text || !style || !valid_font(style->font) || style->spacing > 16u)
        return false;
    const psre_m031_framebuffer_t *fb = psre_m031_get();
    if (!fb->available) return false;
    int64_t x = style->x, y = style->y;
    int64_t origin_x = x;
    uint8_t height = font_height(style->font);
    int64_t right = style->width ? (int64_t)style->x + style->width : (int64_t)fb->width;
    int64_t clip_h = style->height;
    /* A one-line caller box sized for 8 px text must still show a full large line. */
    if (is_large(style->font) && clip_h && clip_h < (int64_t)LARGE_ROWS + 1) clip_h = (int64_t)LARGE_ROWS + 1;
    int64_t bottom = clip_h ? (int64_t)style->y + clip_h : (int64_t)fb->height;
    int64_t clip_x0 = style->x > 0 ? style->x : 0;
    int64_t clip_y0 = style->y > 0 ? style->y : 0;
    int64_t clip_x1 = right < (int64_t)fb->width ? right : (int64_t)fb->width;
    int64_t clip_y1 = bottom < (int64_t)fb->height ? bottom : (int64_t)fb->height;
    if (clip_x1 <= clip_x0 || clip_y1 <= clip_y0) return *text == '\0';
    bool any = false;
    for (const char *p=text; *p; ++p) {
        unsigned char ch=(unsigned char)*p;
        if (ch=='\n') { x=origin_x; y += height; }
        else if (ch=='\r') { x=origin_x; }
        else if (ch=='\t') { x += (int64_t)(8u+style->spacing)*4; }
        else {
            if (x + 8 > right) { x=origin_x; y += height; }
            if (y >= bottom || y >= (int64_t)fb->height) break;
            if (x > INT32_MAX || x < INT32_MIN || y > INT32_MAX || y < INT32_MIN) return false;
            if (!draw_glyph((int32_t)x,(int32_t)y,(char)ch,style->font,
                            style->foreground,style->background,style->opaque_background,
                            clip_x0,clip_y0,clip_x1,clip_y1)) return false;
            any=true;
            x += 8u + style->spacing;
        }
        if (y >= bottom || y >= (int64_t)fb->height) break;
    }
    return any || *text == '\0';
}

psre_m033_text_size_t psre_m033_measure(const char *text, psre_m033_font_t font, uint8_t spacing) {
    psre_m033_text_size_t result={0,0};
    if (!text || !valid_font(font) || spacing > 16u) return result;
    uint32_t line_width=0, max_width=0, lines=1;
    for (const char *p=text; *p; ++p) {
        if (*p=='\n') { if(line_width>max_width)max_width=line_width; line_width=0; ++lines; }
        else if (*p=='\r') line_width=0;
        else if (*p=='\t') line_width += (8u+spacing)*4u;
        else line_width += 8u+spacing;
    }
    if(line_width>max_width)max_width=line_width;
    if(max_width && max_width>=8u+spacing) max_width-=spacing;
    result.width=max_width;
    result.height=lines*font_height(font);
    return result;
}

void psre_m033_report(void) {
    serial_write("[M033] Font Rendering Engine: ");
    serial_write(g_initialized?"ready":"not initialized");
    serial_write(g_ui_large?" | UI size: large (8x16 glyphs)":" | UI size: normal (8x8 glyphs)");
    serial_write(" | fonts: 8x8, 8x16 | ASCII: 0x20-0x7E | chars drawn: ");
    char b[21]; unsigned n=0; uint64_t v=g_chars_drawn;
    do { b[n++]=(char)('0'+v%10u); v/=10u; } while(v && n<sizeof(b));
    while(n) serial_write_char(b[--n]);
    serial_write("\n");
}

bool psre_m033_draw_demo(void) {
    if (!g_initialized) return false;
    const psre_m031_framebuffer_t *fb=psre_m031_get();
    if(!fb->available || fb->width<160u || fb->height<80u) return false;
    psre_m032_color_t black={12,18,30}, white={240,244,250}, cyan={60,210,230};
    psre_m032_color_t muted={160,180,200}, green={70,220,130};
    if(!psre_m032_clear(black)) return false;
    psre_m032_fill_rect(0,0,fb->width,28,cyan);
    psre_m033_text_style_t title={.x=12,.y=6,.width=(uint32_t)fb->width-24,.height=16,
        .spacing=1,.font=PSRE_M033_FONT_8X16,.foreground=black,.background=cyan,.opaque_background=false};
    psre_m033_text_style_t body={.x=16,.y=48,.width=(uint32_t)fb->width-32,.height=(uint32_t)fb->height-56,
        .spacing=1,.font=PSRE_M033_FONT_8X16,.foreground=white,.background=black,.opaque_background=false};
    psre_m033_text_style_t small={.x=16,.y=32,.width=(uint32_t)fb->width-32,.height=12,
        .spacing=1,.font=PSRE_M033_FONT_8X8,.foreground=muted,.background=black,.opaque_background=false};
    bool ok=psre_m033_draw_string("PSRE GRAPHICS CONSOLE",&title);
    ok=psre_m033_draw_string("Portable System Recovery Environment",&small)&&ok;
    ok=psre_m033_draw_string("M033 FONT RENDERING: READY\nASCII CHARACTER SET: 95 GLYPHS\nFONT MODES: 8x8 / 8x16\nGRAPHICS BACKEND: M032 / M031\nDISPLAY STATUS: ONLINE",&body)&&ok;
    psre_m032_draw_line(12,44,(int32_t)fb->width-12,44,green);
    return ok;
}
