#include <psre/m035/shell.h>
#include <psre/m036/app.h>
#include <psre/m037/widget.h>
#include <psre/m038/container.h>
#include <psre/m039/layout.h>
#include <psre/m040/controls.h>
#include <psre/m041/recovery_manager.h>
#include <psre/m042/file_explorer.h>
#include <psre/m061/terminal.h>
#include <psre/m030/input.h>
#include <psre/m031/framebuffer.h>
#include <psre/m032/graphics.h>
#include <psre/m033/font.h>
#include <psre/m003/pit.h>
#include <psre/serial.h>

#define TASKBAR_HEIGHT 30
#define START_WIDTH 78
#define MENU_WIDTH 224
#define MENU_ROW_HEIGHT 30
#define MENU_ITEMS 6u

static psre_m035_status_t status;
static psre_m030_subscription_t subscription;
static psre_m036_app_id_t app_ids[PSRE_M035_APP_COUNT];
static psre_m038_container_id_t app_containers[PSRE_M035_APP_COUNT];
static uint32_t widget_action_count;
static void shell_widget_action(psre_m038_container_id_t container,psre_m038_widget_id_t widget,void *context) {
    (void)container; (void)widget; (void)context; ++widget_action_count;
}
static const char *app_titles[PSRE_M035_APP_COUNT]={
    "PSRE Recovery Manager","System Information","USB Device Manager","PSRE Settings",
    "PSRE File Explorer","PSRE Terminal"
};
static const char *app_contents[PSRE_M035_APP_COUNT]={
    "RECOVERY MANAGER\n\nRAM workspace: M008\nMulti-file queue: M009\nSource media: READ ONLY\n\nSerial CLI: recover PATH\nqueue-add PATH\nqueue-export",
    "SYSTEM INFORMATION\n\nArchitecture: x86_64\nBoot: UEFI / Limine\nGraphics: M031-M033\nWindow Manager: M034\nGUI Shell: M035\nApplication Framework: M036\n\nClock: system uptime",
    "USB DEVICE MANAGER\n\nHost controller: xHCI foundation\nHID keyboard/mouse: M023-M030\n\nUSB Mass Storage: NOT IMPLEMENTED\nDevice inventory UI: foundation only",
    "PSRE SETTINGS\n\nDisplay: Limine framebuffer\nTheme: Dark blue\nInput: USB HID via M030\nWindow system: M034\nApplication Framework: M036\n\nConfiguration persistence: planned",
    "PSRE FILE EXPLORER\n\nFAT32 read-only browser\n8.3 path navigation\nFile metadata and size\nRecovery staging to RAM\nMulti-file recovery queue",
    ""
};

static psre_m032_color_t color(uint8_t r,uint8_t g,uint8_t b) {
    psre_m032_color_t c={r,g,b}; return c;
}
static bool text(const char *s,int32_t x,int32_t y,uint32_t w,uint32_t h,
                 psre_m032_color_t fg) {
    psre_m033_text_style_t st={
        .x=x,.y=y,.width=w,.height=h,.spacing=1,.font=PSRE_M033_FONT_8X8,
        .foreground=fg,.background={0,0,0},.opaque_background=false
    };
    return psre_m033_draw_string(s,&st);
}
static uint32_t screen_width(void) {
    const psre_m031_framebuffer_t *fb=psre_m031_get();
    return fb && fb->available ? (uint32_t)fb->width : 0u;
}
static uint32_t screen_height(void) {
    const psre_m031_framebuffer_t *fb=psre_m031_get();
    return fb && fb->available ? (uint32_t)fb->height : 0u;
}
static void make_clock(char out[9]) {
    uint64_t seconds=psre_pit_ticks()/100u;
    uint32_t h=(uint32_t)((seconds/3600u)%100u);
    uint32_t m=(uint32_t)((seconds/60u)%60u);
    uint32_t s=(uint32_t)(seconds%60u);
    out[0]=(char)('0'+h/10u); out[1]=(char)('0'+h%10u);
    out[2]=':'; out[3]=(char)('0'+m/10u); out[4]=(char)('0'+m%10u);
    out[5]=':'; out[6]=(char)('0'+s/10u); out[7]=(char)('0'+s%10u);
    out[8]=0;
    status.uptime_seconds=seconds;
}
static bool draw_icon(int32_t x,int32_t y,const char *label,uint8_t kind) {
    psre_m032_color_t tile=kind==0?color(35,145,190):
                           kind==1?color(55,155,105):
                           kind==2?color(160,115,55):
                           kind==4?color(24,28,36):color(105,100,175);
    bool ok=psre_m032_fill_rect(x,y,42,34,tile);
    ok=psre_m032_draw_rect(x,y,42,34,color(220,230,240))&&ok;
    if(kind==0) {
        ok=psre_m032_fill_rect(x+9,y+7,24,18,color(235,245,250))&&ok;
        ok=psre_m032_draw_line(x+13,y+12,x+29,y+12,color(35,145,190))&&ok;
        ok=psre_m032_draw_line(x+13,y+17,x+25,y+17,color(35,145,190))&&ok;
    } else if(kind==1) {
        ok=psre_m032_draw_circle(x+21,y+17,10,color(245,245,245))&&ok;
        ok=psre_m032_draw_line(x+21,y+8,x+21,y+26,color(245,245,245))&&ok;
    } else if(kind==4) {
        ok=text(">_",x+13,y+13,24,12,color(120,235,150))&&ok;
    } else if(kind==2) {
        ok=psre_m032_fill_rect(x+8,y+8,26,18,color(245,220,160))&&ok;
        ok=psre_m032_draw_rect(x+8,y+8,26,18,color(75,55,35))&&ok;
    } else {
        ok=psre_m032_draw_circle(x+21,y+17,10,color(245,245,245))&&ok;
        ok=psre_m032_draw_circle(x+21,y+17,4,color(245,245,245))&&ok;
    }
    {   /* centered under the 42 px tile; a 12-char label needs 107 px (old 68 px box dropped letters) */
        uint32_t lw=psre_m033_measure(label,PSRE_M033_FONT_8X8,1).width;
        int32_t lx=x+21-(int32_t)lw/2; if(lx<2)lx=2;
        ok=text(label,lx,y+38,lw+10u,12,color(245,248,252))&&ok;
    }
    return ok;
}

/* Called by M034 after clearing the framebuffer and before painting windows. */
bool psre_m035_draw_desktop(void) {
    if(!status.initialized || !psre_m031_get()->available)return false;
    const psre_m031_framebuffer_t *fb=psre_m031_get();
    uint32_t h=(uint32_t)fb->height;
    bool ok=true;
    /* Restrained dark-blue wallpaper with a geometric accent. */
    ok=psre_m032_fill_rect(0,0,(int32_t)fb->width,(int32_t)fb->height,
                           color(22,39,64))&&ok;
    ok=psre_m032_fill_circle((int32_t)(fb->width*3u/4u),
                             (int32_t)(fb->height/3u),
                             (int32_t)(fb->height/5u),color(29,55,86))&&ok;
    ok=text("PORTABLE SYSTEM RECOVERY ENVIRONMENT",18,14,
            (uint32_t)fb->width-36u,12,color(185,205,225))&&ok;
    if(fb->width>=180u && h>TASKBAR_HEIGHT+70u) {
        ok=draw_icon(24,54,"Recovery",0)&&ok;
        ok=draw_icon(24,132,"System Info",1)&&ok;
        if(h>260u) ok=draw_icon(24,210,"USB Devices",2)&&ok;
        if(h>340u) ok=draw_icon(24,288,"Terminal",4)&&ok;
    }
    return ok;
}
static bool draw_taskbar_and_menu(void) {
    uint32_t w=screen_width(),h=screen_height();
    if(w<100u || h<TASKBAR_HEIGHT+20u)return false;
    int32_t y=(int32_t)h-TASKBAR_HEIGHT;
    bool ok=psre_m032_fill_rect(0,y,(int32_t)w,TASKBAR_HEIGHT,color(12,20,34));
    ok=psre_m032_draw_line(0,y,(int32_t)w-1,y,color(65,105,145))&&ok;
    ok=psre_m032_fill_rect(7,y+4,START_WIDTH,22,
                           status.start_menu_open?color(45,115,155):color(34,75,105))&&ok;
    ok=text("PSRE",25,y+11,50,12,color(255,255,255))&&ok;
    /* Taskbar quick-launch indicators. */
    ok=psre_m032_fill_rect(96,y+7,18,16,color(35,145,190))&&ok;
    ok=psre_m032_fill_rect(122,y+7,18,16,color(55,155,105))&&ok;
    ok=psre_m032_fill_rect(148,y+7,18,16,color(160,115,55))&&ok;
    ok=psre_m032_fill_rect(174,y+7,18,16,color(24,28,36))&&ok;
    ok=psre_m032_draw_rect(174,y+7,18,16,color(120,235,150))&&ok;
    ok=text(">",179,y+11,10,12,color(120,235,150))&&ok;
    char clock[9];make_clock(clock);
    int32_t clock_x=(int32_t)w-88;
    ok=text(clock,clock_x,y+(psre_m033_ui_large()?8:11),80,12,color(230,238,248))&&ok;

    if(status.start_menu_open) {
        int32_t menu_y=y-(int32_t)(MENU_ROW_HEIGHT*MENU_ITEMS)-8;
        if(menu_y<4)menu_y=4;
        ok=psre_m032_fill_rect(8,menu_y,MENU_WIDTH,
                               (int32_t)(MENU_ROW_HEIGHT*MENU_ITEMS+8u),
                               color(20,34,53))&&ok;
        ok=psre_m032_draw_rect(8,menu_y,MENU_WIDTH,
                               (int32_t)(MENU_ROW_HEIGHT*MENU_ITEMS+8u),
                               color(85,135,175))&&ok;
        static const char *labels[MENU_ITEMS]={
            "Recovery Manager","System Information","USB Device Manager","Settings","File Explorer","Terminal"
        };
        for(uint8_t i=0;i<MENU_ITEMS;++i) {
            int32_t row_y=menu_y+4+(int32_t)i*MENU_ROW_HEIGHT;
            if(i==status.selected_menu_item)
                ok=psre_m032_fill_rect(13,row_y,MENU_WIDTH-10,MENU_ROW_HEIGHT-2,
                                       color(43,91,132))&&ok;
            ok=text(labels[i],23,row_y+10,MENU_WIDTH-30,12,
                    color(242,246,250))&&ok;
        }
    }
    ++status.redraws;
    return ok;
}
/* Painted by M034 on every render (after windows, before the pointer): application controls and
   content first, then taskbar and Start menu on top. */
static bool shell_overlay(void) {
    (void)psre_m039_reflow_all();
    bool ok=psre_m038_render();
    ok=psre_m040_render()&&ok;
    ok=psre_m061_render()&&ok;
    ok=draw_taskbar_and_menu()&&ok;
    return ok;
}
bool psre_m035_render(void) {
    if(!status.initialized)return false;
    return psre_m034_render();
}
static void shell_input_callback(const psre_input_event_t *event,void *context) {
    (void)context;
    bool shell_handled=psre_m035_handle_event(event);
    if(!shell_handled) {
        bool advanced_handled=psre_m040_handle_event(event);
        (void)psre_m042_handle_event(event);
        if(!advanced_handled && !psre_m038_handle_event(event))
            (void)psre_m037_handle_event(event);
    }
    (void)psre_m036_dispatch_event(event);
    (void)psre_m036_reconcile();
    (void)psre_m038_reconcile();
}
bool psre_m035_initialize(void) {
    if(status.initialized)return true;
    const psre_m030_status_t *input=psre_m030_get_status();
    const psre_m034_status_t *wm=psre_m034_get_status();
    const psre_m031_framebuffer_t *fb=psre_m031_get();
    if(!input || !input->initialized || !wm || !wm->initialized ||
       !fb || !fb->available || !psre_m032_initialize() || !psre_m033_initialize()) {
        serial_write("[M035] Requires initialized M030/M034 and a valid framebuffer.\n");
        return false;
    }
    status=(psre_m035_status_t){0};
    psre_m033_set_ui_large(true);          /* desktop UI text: larger, easier to read */
    if(!psre_m036_initialize() || !psre_m037_initialize() || !psre_m038_initialize() || !psre_m039_initialize() || !psre_m040_initialize())return false;
    for(uint8_t i=0;i<PSRE_M035_APP_COUNT;++i)app_containers[i]=PSRE_M038_INVALID_CONTAINER;
    for(uint8_t i=0;i<PSRE_M035_APP_COUNT;++i) {
        psre_m036_app_descriptor_t d={.title=app_titles[i],.initial_text=app_contents[i],
                                      .on_event=(i==PSRE_M035_APP_TERMINAL)?psre_m061_on_event:0};
        app_ids[i]=psre_m036_register(&d);
        if(app_ids[i]==PSRE_M036_INVALID_APP)return false;
    }
    if(!psre_m034_set_desktop_painter(psre_m035_draw_desktop) || !psre_m034_set_overlay_painter(shell_overlay))return false;
    status.initialized=true;
    if(!psre_m030_subscribe(PSRE_M030_ANY_EVENT,shell_input_callback,0,&subscription)) {
        status.initialized=false;
        serial_write("[M035] Unable to subscribe to M030 events.\n");
        return false;
    }
    serial_write("[M035] GUI shell initialized.\n");
    return psre_m035_render();
}
static bool point_in(int32_t x,int32_t y,int32_t rx,int32_t ry,int32_t rw,int32_t rh) {
    return x>=rx && y>=ry && x<rx+rw && y<ry+rh;
}
void psre_m035_toggle_start_menu(void) {
    if(!status.initialized)return;
    status.start_menu_open=!status.start_menu_open;
    if(status.start_menu_open)status.selected_menu_item=0;
    (void)psre_m034_render();
}
bool psre_m035_launch_app(psre_m035_app_t app) {
    if(!status.initialized || app>=PSRE_M035_APP_COUNT)return false;
    uint32_t sw=screen_width(),sh=screen_height();
    int32_t ww=sw>600u?520:(int32_t)sw-40;
    int32_t wh=sh>420u?300:(int32_t)sh-80;
    if(app==PSRE_M035_APP_TERMINAL) {            /* terminal gets a larger window */
        ww=sw>720u?680:(int32_t)sw-40;
        wh=sh>520u?400:(int32_t)sh-80;
    }
    if(ww<96)ww=96;
    if(wh<64)wh=64;
    int32_t x=((int32_t)sw-ww)/2;
    int32_t y=((int32_t)sh-wh-TASKBAR_HEIGHT)/2;
    if(!psre_m036_launch(app_ids[app],x,y,ww,wh))return false;
    if(app==PSRE_M035_APP_TERMINAL) {            /* custom-drawn content: no widget container */
        if(!psre_m061_open(app_ids[app])) { (void)psre_m036_stop(app_ids[app]); return false; }
        ++status.app_launches;
        status.start_menu_open=false;
        (void)psre_m035_render();
        return true;
    }
    app_containers[app]=psre_m038_create_container(app_ids[app]);
    if(app_containers[app]==PSRE_M038_INVALID_CONTAINER) {
        (void)psre_m036_stop(app_ids[app]);
        return false;
    }
    psre_m034_window_info_t app_window;
    psre_m038_container_info_t container_info;
    if(app!=PSRE_M035_APP_RECOVERY && app!=PSRE_M035_APP_FILE_EXPLORER &&
       psre_m038_get_container(app_containers[app],&container_info) &&
       container_info.widget_count==0u) {
        psre_m036_app_info_t app_info;
        if(psre_m036_get_app(app_ids[app],&app_info) &&
           psre_m034_get_window(app_info.window,&app_window) &&
           app_window.width>=360 && app_window.height>=220) {
            /* Large text needs more body height; anchor the controls to the window bottom then. */
            int32_t base=132;
            if(psre_m033_ui_large()) { int32_t b=app_window.height-26-86; if(b>base)base=b; }
            (void)psre_m038_add_label(app_containers[app],"Window-scoped controls",10,base,220);
            (void)psre_m038_add_button(app_containers[app],"Refresh",10,base+26,96,28,
                                      shell_widget_action,0);
            (void)psre_m038_add_checkbox(app_containers[app],"Enabled",120,base+30,150,true);
        }
    }
    if(app==PSRE_M035_APP_RECOVERY &&
       !psre_m041_initialize(app_containers[app])) {
        (void)psre_m036_stop(app_ids[app]);
        (void)psre_m038_destroy_container(app_containers[app]);
        app_containers[app]=PSRE_M038_INVALID_CONTAINER;
        return false;
    }
    if(app==PSRE_M035_APP_FILE_EXPLORER &&
       !psre_m042_initialize(app_containers[app])) {
        (void)psre_m036_stop(app_ids[app]);
        (void)psre_m038_destroy_container(app_containers[app]);
        app_containers[app]=PSRE_M038_INVALID_CONTAINER;
        return false;
    }
    ++status.app_launches;
    status.start_menu_open=false;
    (void)psre_m035_render();
    return true;
}

bool psre_m035_handle_event(const psre_input_event_t *event) {
    if(!status.initialized || !event)return false;
    ++status.input_events;
    if(event->type==PSRE_INPUT_KEY_DOWN && event->code==0x17u &&      /* Ctrl+Alt+T opens the terminal */
       (event->modifiers&0x11u) && (event->modifiers&0x44u))
        return psre_m035_launch_app(PSRE_M035_APP_TERMINAL);
    if(event->type==PSRE_INPUT_KEY_DOWN || event->type==PSRE_INPUT_KEY_REPEAT) {
        if(status.start_menu_open) {
            if(event->code==0x52u) {
                status.selected_menu_item=(uint8_t)((status.selected_menu_item+MENU_ITEMS-1u)%MENU_ITEMS);
                (void)psre_m034_render();return true;
            }
            if(event->code==0x51u) {
                status.selected_menu_item=(uint8_t)((status.selected_menu_item+1u)%MENU_ITEMS);
                (void)psre_m034_render();return true;
            }
            if(event->character==0x1bu) {
                status.start_menu_open=false;(void)psre_m034_render();return true;
            }
            if(event->character=='\n')return psre_m035_launch_app((psre_m035_app_t)status.selected_menu_item);
        } else if(event->character==0x1bu) {
            status.start_menu_open=false;(void)psre_m034_render();return false;
        }
    }
    if(event->type==PSRE_INPUT_MOUSE_BUTTON_DOWN && event->code==1u) {
        const psre_m034_status_t *wm=psre_m034_get_status();
        if(!wm)return false;
        int32_t x=wm->cursor_x,y=wm->cursor_y;
        uint32_t h=screen_height();
        int32_t task_y=(int32_t)h-TASKBAR_HEIGHT;
        if(point_in(x,y,7,task_y+3,START_WIDTH,24)) {
            psre_m035_toggle_start_menu();return true;
        }
        if(status.start_menu_open) {
            int32_t menu_y=task_y-(int32_t)(MENU_ROW_HEIGHT*MENU_ITEMS)-8;
            if(menu_y<4)menu_y=4;
            if(point_in(x,y,8,menu_y,MENU_WIDTH,(int32_t)(MENU_ROW_HEIGHT*MENU_ITEMS+8u))) {
                int32_t row=(y-menu_y-4)/MENU_ROW_HEIGHT;
                if(row>=0 && row<(int32_t)MENU_ITEMS)
                    return psre_m035_launch_app((psre_m035_app_t)row);
            } else {
                status.start_menu_open=false;
            }
        }
        if(point_in(x,y,24,54,68,58))return psre_m035_launch_app(PSRE_M035_APP_RECOVERY);
        if(point_in(x,y,24,132,68,58))return psre_m035_launch_app(PSRE_M035_APP_SYSTEM_INFO);
        if(point_in(x,y,24,210,68,58))return psre_m035_launch_app(PSRE_M035_APP_USB_MANAGER);
        if(screen_height()>340u && point_in(x,y,24,288,68,58))return psre_m035_launch_app(PSRE_M035_APP_TERMINAL);
        if(point_in(x,y,94,task_y+4,22,22))return psre_m035_launch_app(PSRE_M035_APP_RECOVERY);
        if(point_in(x,y,120,task_y+4,22,22))return psre_m035_launch_app(PSRE_M035_APP_SYSTEM_INFO);
        if(point_in(x,y,146,task_y+4,22,22))return psre_m035_launch_app(PSRE_M035_APP_USB_MANAGER);
        if(point_in(x,y,172,task_y+4,22,22))return psre_m035_launch_app(PSRE_M035_APP_TERMINAL);
        (void)psre_m034_render();
    }
    return false;
}
const psre_m035_status_t *psre_m035_get_status(void){return &status;}
void psre_m035_report(void) {
    serial_write("\n--- M035 GUI SHELL ---\nInitialized=");
    serial_write(status.initialized?"yes":"no");
    serial_write(" start_menu=");serial_write(status.start_menu_open?"open":"closed");
    serial_write(" selected=");serial_write_char((char)('0'+status.selected_menu_item));
    serial_write(" app_launches=");
    char b[11];uint8_t n=0;uint32_t v=status.app_launches;
    do{b[n++]=(char)('0'+v%10u);v/=10u;}while(v&&n<sizeof(b));
    while(n)serial_write_char(b[--n]);
    serial_write(" uptime_seconds=");
    uint64_t sec=psre_pit_ticks()/100u;
    char c[21];n=0;do{c[n++]=(char)('0'+sec%10u);sec/=10u;}while(sec&&n<sizeof(c));
    while(n)serial_write_char(c[--n]);
    serial_write("\n");
}
