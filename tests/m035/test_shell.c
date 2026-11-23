#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <psre/m035/shell.h>
#include <psre/m038/container.h>
#include <psre/m042/file_explorer.h>
#include <psre/m041/recovery_manager.h>
#include <psre/m030/input.h>
#include <psre/m031/framebuffer.h>
#include <psre/m032/graphics.h>
#include <psre/m033/font.h>
#include <psre/m003/pit.h>
#include <psre/serial.h>

static psre_m031_framebuffer_t fb;
static psre_m030_status_t input_status;
static psre_m034_status_t wm_status;
static psre_m030_event_callback_t saved_callback;
static psre_m034_desktop_painter_t saved_painter;
static void *saved_context;
static unsigned draws, launches;
static psre_m034_window_id_t next_id;
static psre_m034_window_info_t win[8];

bool psre_m041_initialize(psre_m038_container_id_t container){(void)container;return true;}
bool psre_m042_initialize(psre_m038_container_id_t container){(void)container;return true;}
bool psre_m042_handle_event(const psre_input_event_t *event){(void)event;return false;}
const psre_m030_status_t *psre_m030_get_status(void){return &input_status;}
bool psre_m030_subscribe(uint8_t type,psre_m030_event_callback_t cb,void *ctx,psre_m030_subscription_t *out){
    assert(type==PSRE_M030_ANY_EVENT && cb && out);saved_callback=cb;saved_context=ctx;*out=1;return true;
}
const psre_m031_framebuffer_t *psre_m031_get(void){return &fb;}
bool psre_m032_initialize(void){return fb.available;}
bool psre_m033_initialize(void){return fb.available;}
static bool ui_large_flag;
void psre_m033_set_ui_large(bool l){ui_large_flag=l;}
bool psre_m033_ui_large(void){return ui_large_flag;}
psre_m033_text_size_t psre_m033_measure(const char *t,psre_m033_font_t f,uint8_t sp){(void)f;psre_m033_text_size_t r={0,8};while(t&&*t++)r.width+=8u+sp;return r;}
bool psre_m032_clear(psre_m032_color_t c){(void)c;++draws;return true;}
bool psre_m032_fill_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c){(void)x;(void)y;(void)w;(void)h;(void)c;++draws;return true;}
bool psre_m032_draw_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c){(void)x;(void)y;(void)w;(void)h;(void)c;++draws;return true;}
bool psre_m032_draw_line(int32_t x,int32_t y,int32_t a,int32_t b,psre_m032_color_t c){(void)x;(void)y;(void)a;(void)b;(void)c;++draws;return true;}
bool psre_m032_fill_circle(int32_t x,int32_t y,int32_t r,psre_m032_color_t c){(void)x;(void)y;(void)r;(void)c;++draws;return true;}
bool psre_m032_draw_circle(int32_t x,int32_t y,int32_t r,psre_m032_color_t c){(void)x;(void)y;(void)r;(void)c;++draws;return true;}
bool psre_m032_draw_pixel(int32_t x,int32_t y,psre_m032_color_t c){(void)x;(void)y;(void)c;++draws;return true;}
bool psre_m032_draw_triangle(int32_t a,int32_t b,int32_t c,int32_t d,int32_t e,int32_t f,psre_m032_color_t g){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;++draws;return true;}
bool psre_m032_fill_triangle(int32_t a,int32_t b,int32_t c,int32_t d,int32_t e,int32_t f,psre_m032_color_t g){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;++draws;return true;}
bool psre_m032_draw_bitmap(int32_t x,int32_t y,const psre_m032_bitmap_t *b){(void)x;(void)y;(void)b;++draws;return true;}
bool psre_m032_clip_rect(psre_m032_rect_t *r){return r!=0;}

bool psre_m033_draw_string(const char *s,const psre_m033_text_style_t *st){assert(s&&st);++draws;return true;}
uint64_t psre_pit_ticks(void){return 123400u;}
void serial_write(const char *s){(void)s;}
void serial_write_char(char c){(void)c;}

const psre_m034_status_t *psre_m034_get_status(void){return &wm_status;}
static psre_m034_desktop_painter_t saved_overlay;
bool psre_m034_set_desktop_painter(psre_m034_desktop_painter_t p){saved_painter=p;return p!=0;}
bool psre_m034_set_overlay_painter(psre_m034_desktop_painter_t p){saved_overlay=p;return p!=0;}
bool psre_m034_render(void){++draws;if(saved_painter)(void)saved_painter();if(saved_overlay)(void)saved_overlay();return true;}
/* M061 terminal is covered by tests/m061 */
bool psre_m061_open(psre_m036_app_id_t a){(void)a;return true;}
bool psre_m061_on_event(psre_m036_app_id_t a,const psre_input_event_t *e,void *s){(void)a;(void)e;(void)s;return false;}
bool psre_m061_render(void){return true;}
psre_m034_window_id_t psre_m034_create_window(const char *title,int32_t x,int32_t y,int32_t w,int32_t h){
    (void)title;
    if(next_id>=8)return PSRE_M034_INVALID_WINDOW;
    psre_m034_window_id_t id=next_id++;win[id]=(psre_m034_window_info_t){.id=id,.x=x,.y=y,.width=w,.height=h,.visible=true};return id;
}
bool psre_m034_set_text(psre_m034_window_id_t id,const char *s){if(id>=8)return false;strncpy(win[id].text,s,sizeof(win[id].text)-1);++launches;return true;}
bool psre_m034_get_window(psre_m034_window_id_t id,psre_m034_window_info_t *out){if(id>=next_id)return false;*out=win[id];return true;}
bool psre_m034_focus(psre_m034_window_id_t id){if(id>=next_id)return false;wm_status.focused_window=id;return true;}
bool psre_m034_destroy_window(psre_m034_window_id_t id){if(id>=next_id)return false;win[id].id=PSRE_M034_INVALID_WINDOW;return true;}
static void send(psre_input_event_t e){assert(saved_callback);saved_callback(&e,saved_context);}

int main(void){
    fb=(psre_m031_framebuffer_t){.available=true,.width=800,.height=600,.pitch=3200,.bytes_per_pixel=4};
    input_status.initialized=true;wm_status.initialized=true;
    assert(psre_m035_initialize());
    assert(saved_callback && saved_painter);
    assert(psre_m035_get_status()->initialized);
    assert(ui_large_flag);                      /* desktop switches the UI text to the large size */
    assert(psre_m035_draw_desktop());
    assert(psre_m035_render());

    psre_m035_toggle_start_menu();
    assert(psre_m035_get_status()->start_menu_open);
    psre_input_event_t down={.type=PSRE_INPUT_KEY_DOWN,.device=PSRE_INPUT_DEVICE_KEYBOARD,.code=0x51};
    send(down);
    assert(psre_m035_get_status()->selected_menu_item==1);
    psre_input_event_t enter={.type=PSRE_INPUT_KEY_DOWN,.device=PSRE_INPUT_DEVICE_KEYBOARD,.character='\n'};
    send(enter);
    assert(!psre_m035_get_status()->start_menu_open);
    assert(psre_m035_get_status()->app_launches==1);
    assert(launches==1);
    assert(strcmp(win[0].text,"SYSTEM INFORMATION\n\nArchitecture: x86_64\nBoot: UEFI / Limine\nGraphics: M031-M033\nWindow Manager: M034\nGUI Shell: M035\nApplication Framework: M036\n\nClock: system uptime")==0);

    assert(psre_m035_launch_app(PSRE_M035_APP_RECOVERY));
    assert(psre_m035_launch_app(PSRE_M035_APP_USB_MANAGER));
    assert(psre_m035_launch_app(PSRE_M035_APP_SETTINGS));
    assert(psre_m035_get_status()->app_launches==4);
    assert(psre_m038_get_status()->container_count==4u);
    psre_m038_container_info_t first_container;
    assert(psre_m038_get_container(0,&first_container));
    assert(first_container.widget_count==3u);
    assert(psre_m035_get_status()->uptime_seconds==1234u);

    /* Mouse opens Start and selects a menu row. */
    wm_status.cursor_x=20; wm_status.cursor_y=580;
    psre_input_event_t mouse_down={.type=PSRE_INPUT_MOUSE_BUTTON_DOWN,
        .device=PSRE_INPUT_DEVICE_MOUSE,.code=1,.buttons=1};
    send(mouse_down);
    assert(psre_m035_get_status()->start_menu_open);
    wm_status.cursor_x=30; wm_status.cursor_y=521; /* File Explorer row in the six-item menu */
    send(mouse_down);
    assert(!psre_m035_get_status()->start_menu_open);
    assert(psre_m035_get_status()->app_launches==5);

    /* Desktop Recovery icon is a launcher. */
    wm_status.cursor_x=30; wm_status.cursor_y=60;
    send(mouse_down);
    assert(psre_m035_get_status()->app_launches==6);

    /* Sixth Start Menu item launches the Terminal. */
    psre_m035_toggle_start_menu();
    wm_status.cursor_x=30; wm_status.cursor_y=551;
    send(mouse_down);
    assert(psre_m035_get_status()->app_launches==7);
    /* Ctrl+Alt+T, the desktop Terminal icon and the taskbar quick-launch all open the terminal. */
    psre_input_event_t cat={.type=PSRE_INPUT_KEY_DOWN,.device=PSRE_INPUT_DEVICE_KEYBOARD,.code=0x17,.modifiers=0x05};
    send(cat);
    assert(psre_m035_get_status()->app_launches==8);
    wm_status.cursor_x=30; wm_status.cursor_y=295;
    send(mouse_down);
    assert(psre_m035_get_status()->app_launches==9);
    wm_status.cursor_x=180; wm_status.cursor_y=580;
    send(mouse_down);
    assert(psre_m035_get_status()->app_launches==10);
    assert(draws>0);
    puts("M035 GUI shell host tests: PASS");
    return 0;
}
