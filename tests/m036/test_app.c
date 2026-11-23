#include <assert.h>
#include <psre/m033/font.h>
#include <stdio.h>
#include <string.h>
#include <psre/m036/app.h>
static psre_m034_status_t wm;
static psre_m034_window_info_t windows[8];
static uint8_t next_id;
static unsigned event_calls, control_draws;
const psre_m034_status_t *psre_m034_get_status(void){return &wm;}
psre_m034_window_id_t psre_m034_create_window(const char *t,int32_t x,int32_t y,int32_t w,int32_t h){(void)t;(void)x;(void)y;(void)w;(void)h;if(next_id>=8)return PSRE_M034_INVALID_WINDOW;windows[next_id].id=next_id;windows[next_id].visible=true;return next_id++;}
bool psre_m034_set_text(psre_m034_window_id_t id,const char *s){if(id>=next_id)return false;strncpy(windows[id].text,s,sizeof(windows[id].text)-1);return true;}
bool psre_m034_get_window(psre_m034_window_id_t id,psre_m034_window_info_t *o){if(id>=next_id||windows[id].id==PSRE_M034_INVALID_WINDOW)return false;*o=windows[id];return true;}
bool psre_m034_focus(psre_m034_window_id_t id){if(id>=next_id)return false;wm.focused_window=id;return true;}
bool psre_m034_destroy_window(psre_m034_window_id_t id){if(id>=next_id)return false;windows[id].id=PSRE_M034_INVALID_WINDOW;return true;}
static bool on_event(psre_m036_app_id_t id,const psre_input_event_t *e,void *state){(void)id;(void)e;((uint8_t*)state)[0]++;++event_calls;return true;}
void serial_write(const char *s){(void)s;} void serial_write_char(char c){(void)c;}
bool psre_m032_fill_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c){(void)x;(void)y;(void)w;(void)h;(void)c;++control_draws;return true;}
bool psre_m032_draw_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c){(void)x;(void)y;(void)w;(void)h;(void)c;++control_draws;return true;}
bool psre_m032_draw_line(int32_t x,int32_t y,int32_t a,int32_t b,psre_m032_color_t c){(void)x;(void)y;(void)a;(void)b;(void)c;++control_draws;return true;}
bool psre_m033_draw_string(const char *s,const psre_m033_text_style_t *st){(void)s;(void)st;++control_draws;return true;}
int main(void){
    assert(psre_m036_initialize());assert(psre_m036_get_status()->initialized);
    psre_m036_app_descriptor_t d={.title="Test App",.initial_text="Hello",.on_event=on_event};
    psre_m036_app_id_t id=psre_m036_register(&d);assert(id!=PSRE_M036_INVALID_APP);
    assert(psre_m036_register(&d)==id);assert(psre_m036_get_status()->registered_apps==1);
    assert(!psre_m036_launch(9,0,0,200,100));
    assert(psre_m036_launch(id,10,10,240,160));
    psre_m036_app_info_t info;assert(psre_m036_get_app(id,&info));assert(info.lifecycle==PSRE_M036_RUNNING);assert(info.window==0);
    psre_input_event_t ev={.type=PSRE_INPUT_KEY_DOWN,.character='a'};
    assert(psre_m036_dispatch_event(&ev));assert(event_calls==1);
    assert(((uint8_t*)psre_m036_get_state(id))[0]==1);
    assert(psre_m036_stop(id));assert(!psre_m036_stop(id));
    assert(psre_m036_get_app(id,&info));assert(info.lifecycle==PSRE_M036_STOPPED);
    assert(psre_m036_launch(id,10,10,240,160));
    windows[1].id=PSRE_M034_INVALID_WINDOW;
    assert(psre_m036_reconcile());
    assert(psre_m036_get_app(id,&info));assert(info.lifecycle==PSRE_M036_STOPPED);
    assert(psre_m036_label("Status",1,1,80,(psre_m032_color_t){255,255,255}));
    assert(psre_m036_button("OK",1,20,80,24,false));
    assert(psre_m036_textbox("value",1,50,120,24,true));
    assert(psre_m036_checkbox("Enable",1,80,true));
    assert(control_draws>0);
    assert(psre_m036_get_status()->launches==2);
    assert(psre_m036_get_status()->running_apps==0);
    puts("M036 application framework host tests: PASS");return 0;
}
