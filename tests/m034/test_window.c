
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <psre/m034/window.h>
#include <psre/m031/framebuffer.h>
#include <psre/m032/graphics.h>
#include <psre/m033/font.h>
#include <psre/serial.h>

static psre_m031_framebuffer_t fb;
static psre_m030_status_t input_status;
static psre_m030_event_callback_t saved_callback;
static void *saved_context;
static unsigned render_calls;

bool psre_m031_initialize(void) { return true; }
const psre_m031_framebuffer_t *psre_m031_get(void) { return &fb; }
bool psre_m031_put_pixel(uint64_t x,uint64_t y,uint8_t r,uint8_t g,uint8_t b) {
    (void)r;(void)g;(void)b; return x<fb.width && y<fb.height;
}
bool psre_m031_fill_rect(uint64_t x,uint64_t y,uint64_t w,uint64_t h,uint8_t r,uint8_t g,uint8_t b) {
    (void)r;(void)g;(void)b; return x<fb.width && y<fb.height && w && h;
}
bool psre_m032_initialize(void) { return fb.available; }
bool psre_m032_clear(psre_m032_color_t c) { (void)c; ++render_calls; return true; }
bool psre_m032_fill_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c) {
    (void)x;(void)y;(void)w;(void)h;(void)c; ++render_calls; return true;
}
bool psre_m032_draw_line(int32_t x0,int32_t y0,int32_t x1,int32_t y1,psre_m032_color_t c) {
    (void)x0;(void)y0;(void)x1;(void)y1;(void)c; ++render_calls; return true;
}
bool psre_m033_initialize(void) { return true; }
bool psre_m033_draw_string(const char *s,const psre_m033_text_style_t *st) {
    (void)s;(void)st; ++render_calls; return true;
}
bool psre_m030_subscribe(uint8_t type,psre_m030_event_callback_t cb,void *ctx,
                         psre_m030_subscription_t *out) {
    assert(type==PSRE_M030_ANY_EVENT && cb && out);
    saved_callback=cb; saved_context=ctx; *out=0; return true;
}
const psre_m030_status_t *psre_m030_get_status(void) { return &input_status; }
void serial_write(const char *s) { (void)s; }
void serial_write_char(char c) { (void)c; }

static void send_event(psre_input_event_t e) {
    assert(saved_callback); saved_callback(&e,saved_context);
}
static int overlay_calls;
static bool test_overlay(void){++overlay_calls;return true;}
int main(void) {
    fb=(psre_m031_framebuffer_t){.available=true,.width=800,.height=600,.pitch=3200,.bytes_per_pixel=4};
    input_status.initialized=true;
    assert(psre_m034_initialize());
    assert(saved_callback);

    psre_m034_window_id_t ids[8];
    for(unsigned i=0;i<8;i++) {
        ids[i]=psre_m034_create_window("Window",20+(int)i*5,20+(int)i*5,160,100);
        assert(ids[i]!=PSRE_M034_INVALID_WINDOW);
    }
    assert(psre_m034_create_window("Ninth",0,0,160,100)==PSRE_M034_INVALID_WINDOW);
    assert(psre_m034_get_status()->window_count==8);
    assert(psre_m034_focus(ids[2]));
    psre_m034_window_info_t wi;
    assert(psre_m034_get_window(ids[2],&wi) && wi.focused);
    assert(psre_m034_set_title(ids[2],"Focused"));
    assert(psre_m034_set_text(ids[2],"initial"));
    assert(psre_m034_append_text(ids[2],'!'));
    assert(psre_m034_get_window(ids[2],&wi) && strcmp(wi.text,"initial!")==0);
    assert(psre_m034_set_position(ids[2],-50,-40));
    assert(psre_m034_get_window(ids[2],&wi) && wi.x==0 && wi.y==0);
    assert(psre_m034_set_size(ids[2],200,120));
    assert(!psre_m034_set_size(ids[2],20,20));

    /* Click and drag the focused window's title bar. */
    psre_input_event_t initial_move={.type=PSRE_INPUT_MOUSE_MOVE,.device=PSRE_INPUT_DEVICE_MOUSE,
                                     .delta_x=-380,.delta_y=-290};
    send_event(initial_move);
    psre_input_event_t down={.type=PSRE_INPUT_MOUSE_BUTTON_DOWN,.device=PSRE_INPUT_DEVICE_MOUSE,.code=1,.buttons=1};
    send_event(down);
    assert(psre_m034_get_status()->dragging);
    psre_input_event_t move={.type=PSRE_INPUT_MOUSE_MOVE,.device=PSRE_INPUT_DEVICE_MOUSE,.delta_x=280,.delta_y=240};
    send_event(move);
    assert(psre_m034_get_window(ids[2],&wi));
    assert(wi.x==280 && wi.y==240);
    psre_input_event_t up={.type=PSRE_INPUT_MOUSE_BUTTON_UP,.device=PSRE_INPUT_DEVICE_MOUSE,.code=1,.buttons=0};
    send_event(up);
    assert(!psre_m034_get_status()->dragging);

    /* Keyboard character input and backspace are routed to focused window. */
    psre_input_event_t key={.type=PSRE_INPUT_KEY_DOWN,.device=PSRE_INPUT_DEVICE_KEYBOARD,.character='X'};
    send_event(key);
    assert(psre_m034_get_window(ids[2],&wi));
    assert(strcmp(wi.text,"initial!X")==0);
    key.character='\b'; send_event(key);
    assert(psre_m034_get_window(ids[2],&wi) && strcmp(wi.text,"initial!")==0);

    assert(psre_m034_destroy_window(ids[2]));
    assert(psre_m034_get_status()->window_count==7);
    psre_m034_window_id_t reused=psre_m034_create_window("Reuse",0,0,160,100);
    assert(reused!=PSRE_M034_INVALID_WINDOW);
    assert(psre_m034_focus(ids[7]));
    assert(psre_m034_get_window(ids[7],&wi));
    psre_input_event_t close_move={.type=PSRE_INPUT_MOUSE_MOVE,.device=PSRE_INPUT_DEVICE_MOUSE,
                                   .delta_x=(wi.x+wi.width-8)-psre_m034_get_status()->cursor_x,
                                   .delta_y=(wi.y+10)-psre_m034_get_status()->cursor_y};
    send_event(close_move);
    send_event(down);
    assert(!psre_m034_get_window(ids[7],&wi));
    assert(psre_m034_get_status()->window_count==7);
    assert(render_calls>0);
    /* Overlay painter: runs on every render (also those triggered by input events). */
    assert(psre_m034_set_overlay_painter(test_overlay));
    int before=overlay_calls;
    assert(psre_m034_render()); assert(overlay_calls==before+1);
    psre_input_event_t ov_move={.type=PSRE_INPUT_MOUSE_MOVE,.device=PSRE_INPUT_DEVICE_MOUSE,.delta_x=3,.delta_y=3};
    send_event(ov_move); assert(overlay_calls==before+2);
    assert(psre_m034_set_overlay_painter(0));
    assert(psre_m034_render()); assert(overlay_calls==before+2);
    puts("M034 host tests: PASS");
    return 0;
}
