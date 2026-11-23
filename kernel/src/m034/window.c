#include <psre/m034/window.h>
#include <psre/m031/framebuffer.h>
#include <psre/m032/graphics.h>
#include <psre/m033/font.h>
#include <psre/serial.h>
#include <limits.h>
#include <stddef.h>

#define TITLE_HEIGHT 22
#define BORDER 2
#define CLOSE_SIZE 12
#define WINDOW_MIN_WIDTH 96
#define WINDOW_MIN_HEIGHT 64
#define DESKTOP_R 24
#define DESKTOP_G 34
#define DESKTOP_B 52

typedef struct {
    bool used, visible;
    psre_m034_window_id_t id;
    int32_t x, y, width, height;
    char title[PSRE_M034_TITLE_CAPACITY];
    char text[PSRE_M034_TEXT_CAPACITY];
} window_t;

static window_t windows[PSRE_M034_MAX_WINDOWS];
static psre_m034_window_id_t z_order[PSRE_M034_MAX_WINDOWS];
static uint8_t z_count;
static psre_m034_status_t status;
static psre_m030_subscription_t input_subscription;
static int32_t drag_dx, drag_dy;
static psre_m034_desktop_painter_t desktop_painter;
static psre_m034_desktop_painter_t overlay_painter;
static void input_callback(const psre_input_event_t *event,void *context);

static psre_m032_color_t rgb(uint8_t r,uint8_t g,uint8_t b) { psre_m032_color_t c={r,g,b}; return c; }
static window_t *find_window(psre_m034_window_id_t id) {
    for (uint8_t i=0;i<PSRE_M034_MAX_WINDOWS;++i)
        if (windows[i].used && windows[i].id==id) return &windows[i];
    return 0;
}
static size_t copy_string(char *dst,size_t cap,const char *src) {
    size_t n=0;
    if (!dst || !cap) return 0;
    if (src) while (src[n] && n+1u<cap) { dst[n]=src[n]; ++n; }
    dst[n]=0; return n;
}
static void clamp_position(window_t *w) {
    const psre_m031_framebuffer_t *fb=psre_m031_get();
    if (!fb->available) return;
    int32_t max_x=(fb->width>(uint64_t)w->width)?(int32_t)(fb->width-(uint64_t)w->width):0;
    int32_t max_y=(fb->height>(uint64_t)w->height)?(int32_t)(fb->height-(uint64_t)w->height):0;
    if(w->x<0) w->x=0;
    if(w->y<0) w->y=0;
    if(w->x>max_x) w->x=max_x;
    if(w->y>max_y) w->y=max_y;
}
static void remove_z(uint8_t pos) {
    if(pos>=z_count)return;
    for(uint8_t i=pos;i+1u<z_count;++i)z_order[i]=z_order[i+1u];
    --z_count;
}
static bool move_to_top(psre_m034_window_id_t id) {
    for(uint8_t i=0;i<z_count;++i) if(z_order[i]==id) {
        remove_z(i); z_order[z_count++]=id; return true;
    }
    return false;
}
static void update_focus_flags(void) {
    status.focused_window=(z_count?z_order[z_count-1u]:PSRE_M034_INVALID_WINDOW);
    for(uint8_t i=0;i<PSRE_M034_MAX_WINDOWS;++i)
        windows[i].visible=windows[i].used;
}
static bool text_draw(const char *s,int32_t x,int32_t y,uint32_t w,uint32_t h,
                      psre_m032_color_t fg,psre_m032_color_t bg) {
    psre_m033_text_style_t style={
        .x=x,.y=y,.width=w,.height=h,.spacing=1,.font=PSRE_M033_FONT_8X8,
        .foreground=fg,.background=bg,.opaque_background=false
    };
    return psre_m033_draw_string(s,&style);
}
bool psre_m034_initialize(void) {
    if(status.initialized) return true;
    const psre_m030_status_t *input=psre_m030_get_status();
    const psre_m031_framebuffer_t *fb=psre_m031_get();
    if(!input || !input->initialized || !fb || !fb->available ||
       !psre_m032_initialize() || !psre_m033_initialize()) {
        serial_write("[M034] Requires initialized M030 input and M031 framebuffer.\n");
        return false;
    }
    for(uint8_t i=0;i<PSRE_M034_MAX_WINDOWS;++i) windows[i]=(window_t){0};
    z_count=0; desktop_painter=0; overlay_painter=0; status=(psre_m034_status_t){0};
    status.focused_window=PSRE_M034_INVALID_WINDOW;
    status.cursor_x=(int32_t)(fb->width/2u); status.cursor_y=(int32_t)(fb->height/2u);
    status.initialized=true;
    if(!psre_m030_subscribe(PSRE_M030_ANY_EVENT,input_callback,0,&input_subscription)) {
        status.initialized=false;
        serial_write("[M034] Unable to subscribe to M030 input events.\\n");
        return false;
    }
    serial_write("[M034] Window manager core initialized; M030 input subscribed.\\n");
    return true;
}
static void input_callback(const psre_input_event_t *event,void *context) {
    (void)context;
    (void)psre_m034_handle_event(event);
}
bool psre_m034_render(void) {
    if(!status.initialized || !psre_m031_get()->available)return false;
    bool ok=psre_m032_clear(rgb(DESKTOP_R,DESKTOP_G,DESKTOP_B));
    if(desktop_painter) ok=desktop_painter()&&ok;
    for(uint8_t zi=0;zi<z_count;++zi) {
        window_t *w=find_window(z_order[zi]); if(!w || !w->used)continue;
        bool focused=(zi==z_count-1u);
        psre_m032_color_t frame=focused?rgb(75,190,220):rgb(105,120,140);
        psre_m032_color_t title=focused?rgb(35,105,145):rgb(65,75,90);
        psre_m032_color_t body=rgb(238,241,245), ink=rgb(25,32,42);
        ok=psre_m032_fill_rect(w->x,w->y,w->width,w->height,frame)&&ok;
        ok=psre_m032_fill_rect(w->x+BORDER,w->y+BORDER,w->width-2*BORDER,TITLE_HEIGHT,title)&&ok;
        ok=psre_m032_fill_rect(w->x+BORDER,w->y+TITLE_HEIGHT+BORDER,
                              w->width-2*BORDER,w->height-TITLE_HEIGHT-2*BORDER,body)&&ok;
        int32_t close_x=w->x+w->width-CLOSE_SIZE-5;
        int32_t close_y=w->y+5;
        ok=psre_m032_fill_rect(close_x,close_y,CLOSE_SIZE,CLOSE_SIZE,rgb(190,65,65))&&ok;
        ok=text_draw(w->title,w->x+8,w->y+7,(uint32_t)(w->width-CLOSE_SIZE-20),12,
                     rgb(255,255,255),title)&&ok;
        ok=text_draw(w->text,w->x+10,w->y+TITLE_HEIGHT+10,
                     (uint32_t)(w->width-20),(uint32_t)(w->height-TITLE_HEIGHT-18),
                     ink,body)&&ok;
    }
    if(overlay_painter) ok=overlay_painter()&&ok;
    /* Simple software pointer; rendered last, clipped by M032. */
    ok=psre_m032_draw_line(status.cursor_x,status.cursor_y,
                           status.cursor_x,status.cursor_y+12,rgb(255,255,255))&&ok;
    ok=psre_m032_draw_line(status.cursor_x,status.cursor_y,
                           status.cursor_x+8,status.cursor_y+8,rgb(255,255,255))&&ok;
    ++status.redraws;
    return ok;
}
bool psre_m034_set_overlay_painter(psre_m034_desktop_painter_t painter) {
    if(!status.initialized) return false;
    overlay_painter=painter;
    return true;
}
bool psre_m034_set_desktop_painter(psre_m034_desktop_painter_t painter) {
    if(!status.initialized) return false;
    desktop_painter=painter;
    return true;
}
bool psre_m034_handle_event(const psre_input_event_t *event) {
    if(!status.initialized || !event)return false;
    ++status.events_received;
    if(event->type==PSRE_INPUT_MOUSE_MOVE) {
        status.cursor_x += event->delta_x;
        status.cursor_y += event->delta_y;
        const psre_m031_framebuffer_t *fb=psre_m031_get();
        if(status.cursor_x<0)status.cursor_x=0;
        if(status.cursor_y<0)status.cursor_y=0;
        if(status.cursor_x>=(int32_t)fb->width)status.cursor_x=(int32_t)fb->width-1;
        if(status.cursor_y>=(int32_t)fb->height)status.cursor_y=(int32_t)fb->height-1;
        if(status.dragging) {
            window_t *w=find_window(status.focused_window);
            if(w) { w->x=status.cursor_x-drag_dx; w->y=status.cursor_y-drag_dy; clamp_position(w); }
        }
        (void)psre_m034_render(); return true;
    }
    if(event->type==PSRE_INPUT_MOUSE_BUTTON_DOWN && (event->code==1u || (event->buttons&1u))) {
        for(int i=(int)z_count-1;i>=0;--i) {
            window_t *w=find_window(z_order[i]);
            if(!w || status.cursor_x<w->x || status.cursor_x>=w->x+w->width ||
               status.cursor_y<w->y || status.cursor_y>=w->y+w->height)continue;
            (void)psre_m034_focus(w->id);
            int32_t close_x=w->x+w->width-CLOSE_SIZE-5;
            if(status.cursor_y<w->y+TITLE_HEIGHT && status.cursor_x>=close_x) {
                (void)psre_m034_destroy_window(w->id);
            } else if(status.cursor_y<w->y+TITLE_HEIGHT) {
                status.dragging=true; drag_dx=status.cursor_x-w->x; drag_dy=status.cursor_y-w->y;
            }
            (void)psre_m034_render(); return true;
        }
    }
    if(event->type==PSRE_INPUT_MOUSE_BUTTON_UP && (event->code==1u || !(event->buttons&1u))) {
        status.dragging=false; (void)psre_m034_render(); return true;
    }
    if(event->type==PSRE_INPUT_KEY_DOWN || event->type==PSRE_INPUT_KEY_REPEAT) {
        window_t *w=find_window(status.focused_window);
        if(w && event->character) {
            if(event->character=='\b') {
                size_t n=0; while(w->text[n])++n; if(n)w->text[n-1]=0;
            } else if(event->character=='\n' || event->character=='\t' ||
                      (event->character>=32 && event->character<=126)) {
                (void)psre_m034_append_text(w->id,event->character);
            }
            (void)psre_m034_render(); return true;
        }
    }
    return false;
}
bool psre_m034_focus(psre_m034_window_id_t id) {
    if(!status.initialized || !find_window(id) || !move_to_top(id))return false;
    update_focus_flags(); return true;
}
psre_m034_window_id_t psre_m034_create_window(const char *title,int32_t x,int32_t y,int32_t width,int32_t height) {
    if(!status.initialized || !title || width<WINDOW_MIN_WIDTH || height<WINDOW_MIN_HEIGHT ||
       width>16384 || height>16384 || z_count>=PSRE_M034_MAX_WINDOWS)return PSRE_M034_INVALID_WINDOW;
    const psre_m031_framebuffer_t *fb=psre_m031_get();
    if(width>(int32_t)fb->width || height>(int32_t)fb->height)return PSRE_M034_INVALID_WINDOW;
    uint8_t slot=0; while(slot<PSRE_M034_MAX_WINDOWS && windows[slot].used)++slot;
    if(slot==PSRE_M034_MAX_WINDOWS)return PSRE_M034_INVALID_WINDOW;
    psre_m034_window_id_t id=slot;
    windows[slot]=(window_t){.used=true,.visible=true,.id=id,.x=x,.y=y,.width=width,.height=height};
    copy_string(windows[slot].title,sizeof(windows[slot].title),title);
    windows[slot].text[0]=0; clamp_position(&windows[slot]);
    z_order[z_count++]=id; update_focus_flags();
    ++status.windows_created; status.window_count=z_count;
    (void)psre_m034_render(); return id;
}
bool psre_m034_destroy_window(psre_m034_window_id_t id) {
    if(!status.initialized || !find_window(id))return false;
    for(uint8_t i=0;i<z_count;++i)if(z_order[i]==id){remove_z(i);break;}
    windows[id]=(window_t){0}; status.window_count=z_count;
    status.dragging=false; update_focus_flags(); ++status.windows_destroyed;
    (void)psre_m034_render(); return true;
}
bool psre_m034_set_title(psre_m034_window_id_t id,const char *title) {
    window_t *w=find_window(id); if(!status.initialized||!w||!title)return false;
    copy_string(w->title,sizeof(w->title),title); (void)psre_m034_render(); return true;
}
bool psre_m034_set_position(psre_m034_window_id_t id,int32_t x,int32_t y) {
    window_t *w=find_window(id); if(!status.initialized||!w)return false;
    w->x=x;w->y=y;clamp_position(w);(void)psre_m034_render();return true;
}
bool psre_m034_set_size(psre_m034_window_id_t id,int32_t width,int32_t height) {
    window_t *w=find_window(id); const psre_m031_framebuffer_t *fb=psre_m031_get();
    if(!status.initialized||!w||width<WINDOW_MIN_WIDTH||height<WINDOW_MIN_HEIGHT||
       width>(int32_t)fb->width||height>(int32_t)fb->height)return false;
    w->width=width;w->height=height;clamp_position(w);(void)psre_m034_render();return true;
}
bool psre_m034_set_text(psre_m034_window_id_t id,const char *text) {
    window_t *w=find_window(id);if(!status.initialized||!w||!text)return false;
    copy_string(w->text,sizeof(w->text),text);(void)psre_m034_render();return true;
}
bool psre_m034_append_text(psre_m034_window_id_t id,char ch) {
    window_t *w=find_window(id);if(!status.initialized||!w)return false;
    size_t n=0;while(w->text[n]&&n+1u<sizeof(w->text))++n;
    if(n+1u>=sizeof(w->text))return false;
    w->text[n]=ch;w->text[n+1u]=0;(void)psre_m034_render();return true;
}
bool psre_m034_get_window(psre_m034_window_id_t id,psre_m034_window_info_t *out) {
    window_t *w=find_window(id);if(!status.initialized||!w||!out)return false;
    *out=(psre_m034_window_info_t){.id=w->id,.x=w->x,.y=w->y,.width=w->width,
        .height=w->height,.visible=w->visible,.focused=(id==status.focused_window)};
    copy_string(out->title,sizeof(out->title),w->title);
    copy_string(out->text,sizeof(out->text),w->text);return true;
}
const psre_m034_status_t *psre_m034_get_status(void){return &status;}
void psre_m034_report(void) {
    serial_write("[M034] Window Manager: ");serial_write(status.initialized?"ready":"not initialized");
    serial_write(" windows=");serial_write_char((char)('0'+status.window_count));
    serial_write(" focused_id=");
    if(status.focused_window==PSRE_M034_INVALID_WINDOW)serial_write("none");
    else serial_write_char((char)('0'+status.focused_window));
    serial_write(" dragging=");serial_write(status.dragging?"yes":"no");
    serial_write(" events=");char b[11];uint8_t n=0;uint32_t v=status.events_received;
    do{b[n++]=(char)('0'+v%10u);v/=10u;}while(v&&n<sizeof(b));
    while(n) serial_write_char(b[--n]);
    serial_write("\n");
}
bool psre_m034_draw_demo(void) {
    if(!status.initialized)return false;
    (void)psre_m032_clear(rgb(DESKTOP_R,DESKTOP_G,DESKTOP_B));
    psre_m034_window_id_t a=psre_m034_create_window("PSRE Control Center",40,45,360,250);
    psre_m034_window_id_t b=psre_m034_create_window("System Monitor",180,130,300,190);
    if(a==PSRE_M034_INVALID_WINDOW||b==PSRE_M034_INVALID_WINDOW)return false;
    (void)psre_m034_set_text(a,"Welcome to PSRE.\nWindow manager: active\nGraphics: M031-M033\nInput: M030");
    (void)psre_m034_set_text(b,"System Status: READY\nCPU: x86_64\nBoot: UEFI / Limine\nStorage: recovery mode");
    return psre_m034_render();
}
