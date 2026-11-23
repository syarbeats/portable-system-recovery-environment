#include <psre/m036/app.h>
#include <psre/m032/graphics.h>
#include <psre/m033/font.h>
#include <psre/serial.h>
#include <stddef.h>

#define APP_W 520
#define APP_H 300
static psre_m036_app_info_t apps[PSRE_M036_MAX_APPS];
static psre_m036_app_descriptor_t descriptors[PSRE_M036_MAX_APPS];
static psre_m036_status_t status;
static psre_m032_color_t rgb(uint8_t r,uint8_t g,uint8_t b){psre_m032_color_t c={r,g,b};return c;}
static void copy(char *dst,size_t cap,const char *src){size_t i=0;if(!cap)return;while(src&&src[i]&&i+1u<cap){dst[i]=src[i];++i;}dst[i]=0;}
static bool draw_text(const char *s,int32_t x,int32_t y,uint32_t w,uint32_t h,psre_m032_color_t fg){
    psre_m033_text_style_t st={.x=x,.y=y,.width=w,.height=h,.spacing=1,.font=PSRE_M033_FONT_8X8,.foreground=fg,.background={0,0,0},.opaque_background=false};
    return psre_m033_draw_string(s,&st);
}
bool psre_m036_initialize(void){if(status.initialized)return true;for(size_t i=0;i<PSRE_M036_MAX_APPS;++i){apps[i]=(psre_m036_app_info_t){0};descriptors[i]=(psre_m036_app_descriptor_t){0};}status=(psre_m036_status_t){.initialized=true};return true;}
psre_m036_app_id_t psre_m036_register(const psre_m036_app_descriptor_t *d){
    if(!status.initialized||!d||!d->title||!d->initial_text)return PSRE_M036_INVALID_APP;
    for(uint8_t i=0;i<PSRE_M036_MAX_APPS;++i)if(apps[i].registered&&descriptors[i].title==d->title)return i;
    for(uint8_t i=0;i<PSRE_M036_MAX_APPS;++i)if(!apps[i].registered){descriptors[i]=*d;apps[i]=(psre_m036_app_info_t){.registered=true,.lifecycle=PSRE_M036_REGISTERED,.window=PSRE_M034_INVALID_WINDOW};copy(apps[i].title,sizeof(apps[i].title),d->title);++status.registered_apps;return i;}
    return PSRE_M036_INVALID_APP;
}
bool psre_m036_launch(psre_m036_app_id_t id,int32_t x,int32_t y,int32_t w,int32_t h){
    if(!status.initialized||id>=PSRE_M036_MAX_APPS||!apps[id].registered)return false;
    if(apps[id].lifecycle==PSRE_M036_RUNNING){psre_m034_window_info_t wi;if(psre_m034_get_window(apps[id].window,&wi)){(void)psre_m034_focus(apps[id].window);return true;}apps[id].lifecycle=PSRE_M036_STOPPED;apps[id].window=PSRE_M034_INVALID_WINDOW;if(status.running_apps)--status.running_apps;}
    if(w<96) { w=APP_W; }
    if(h<64) { h=APP_H; }
    psre_m034_window_id_t win=psre_m034_create_window(descriptors[id].title,x,y,w,h);
    if(win==PSRE_M034_INVALID_WINDOW){apps[id].lifecycle=PSRE_M036_ERROR;++status.errors;return false;}
    apps[id].window=win;apps[id].lifecycle=PSRE_M036_RUNNING;++apps[id].launches;++status.launches;++status.running_apps;
    for(size_t i=0;i<PSRE_M036_STATE_CAPACITY;++i)apps[id].state[i]=0;
    if(!psre_m034_set_text(win,descriptors[id].initial_text)){(void)psre_m034_destroy_window(win);apps[id].lifecycle=PSRE_M036_ERROR;apps[id].window=PSRE_M034_INVALID_WINDOW;--status.running_apps;++status.errors;return false;}
    (void)psre_m034_focus(win);return true;
}
bool psre_m036_stop(psre_m036_app_id_t id){if(!status.initialized||id>=PSRE_M036_MAX_APPS||!apps[id].registered||apps[id].lifecycle!=PSRE_M036_RUNNING)return false;
    if(!psre_m034_destroy_window(apps[id].window)){apps[id].lifecycle=PSRE_M036_ERROR;++status.errors;return false;}
    apps[id].window=PSRE_M034_INVALID_WINDOW;apps[id].lifecycle=PSRE_M036_STOPPED;if(status.running_apps)--status.running_apps;++status.stops;return true;
}
bool psre_m036_dispatch_event(const psre_input_event_t *event){if(!status.initialized||!event)return false;
    const psre_m034_status_t *wm=psre_m034_get_status();if(!wm)return false;
    for(uint8_t i=0;i<PSRE_M036_MAX_APPS;++i){if(!apps[i].registered||apps[i].lifecycle!=PSRE_M036_RUNNING||apps[i].window!=wm->focused_window)continue;
        ++apps[i].events;++status.dispatched_events;
        if(descriptors[i].on_event)return descriptors[i].on_event(i,event,apps[i].state);
        return true;
    }
    ++status.dispatch_misses;return false;
}
bool psre_m036_reconcile(void){if(!status.initialized)return false;bool ok=true;
    for(uint8_t i=0;i<PSRE_M036_MAX_APPS;++i)if(apps[i].registered&&apps[i].lifecycle==PSRE_M036_RUNNING){psre_m034_window_info_t wi;if(!psre_m034_get_window(apps[i].window,&wi)){apps[i].window=PSRE_M034_INVALID_WINDOW;apps[i].lifecycle=PSRE_M036_STOPPED;if(status.running_apps)--status.running_apps;++status.stops;} }
    return ok;
}
bool psre_m036_get_app(psre_m036_app_id_t id,psre_m036_app_info_t *out){if(!status.initialized||id>=PSRE_M036_MAX_APPS||!out||!apps[id].registered)return false;*out=apps[id];return true;}
void *psre_m036_get_state(psre_m036_app_id_t id){if(!status.initialized||id>=PSRE_M036_MAX_APPS||!apps[id].registered)return NULL;return apps[id].state;}
const psre_m036_status_t *psre_m036_get_status(void){return &status;}
void psre_m036_report(void){serial_write("[M036] Application Framework: ");serial_write(status.initialized?"ready":"not initialized");serial_write(" registered=");serial_write_char((char)('0'+status.registered_apps));serial_write(" running=");serial_write_char((char)('0'+status.running_apps));serial_write(" launches=");serial_write_char((char)('0'+(status.launches%10u)));serial_write(" events=");serial_write_char((char)('0'+(status.dispatched_events%10u)));serial_write(" errors=");serial_write_char((char)('0'+(status.errors%10u)));serial_write("\n");}
bool psre_m036_label(const char *s,int32_t x,int32_t y,uint32_t w,psre_m032_color_t fg){if(!s||!w)return false;return draw_text(s,x,y,w,16,fg);}
bool psre_m036_button(const char *s,int32_t x,int32_t y,int32_t w,int32_t h,bool pressed){if(!s||w<24||h<16)return false;psre_m032_color_t fill=pressed?rgb(35,91,130):rgb(43,112,153);bool ok=psre_m032_fill_rect(x,y,w,h,fill);ok=psre_m032_draw_rect(x,y,w,h,rgb(190,215,235))&&ok;ok=draw_text(s,x+6,y+(h-8)/2,w-12,12,rgb(255,255,255))&&ok;return ok;}
bool psre_m036_textbox(const char *s,int32_t x,int32_t y,int32_t w,int32_t h,bool focused){if(!s||w<24||h<16)return false;bool ok=psre_m032_fill_rect(x,y,w,h,rgb(245,248,252));ok=psre_m032_draw_rect(x,y,w,h,focused?rgb(45,145,205):rgb(110,125,140))&&ok;ok=draw_text(s,x+5,y+(h-8)/2,w-10,12,rgb(25,35,48))&&ok;return ok;}
bool psre_m036_checkbox(const char *s,int32_t x,int32_t y,bool checked){if(!s)return false;bool ok=psre_m032_draw_rect(x,y,14,14,rgb(210,225,238));if(checked)ok=psre_m032_draw_line(x+3,y+7,x+6,y+11,rgb(65,190,125))&&psre_m032_draw_line(x+6,y+11,x+12,y+2,rgb(65,190,125))&&ok;ok=draw_text(s,x+21,y+3,240,12,rgb(235,242,250))&&ok;return ok;}
