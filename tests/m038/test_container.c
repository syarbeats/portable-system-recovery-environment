#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <psre/m038/container.h>
#include <psre/m034/window.h>
#include <psre/m032/graphics.h>
#include <psre/m033/font.h>
static psre_m034_status_t wm;
static psre_m034_window_info_t windows[2];
static psre_m036_app_info_t apps[2];
static unsigned draw_calls,actions;static int32_t last_rect_x;
const psre_m034_status_t *psre_m034_get_status(void){return &wm;}
bool psre_m034_get_window(psre_m034_window_id_t id,psre_m034_window_info_t*out){if(id>=2||!windows[id].visible)return false;*out=windows[id];return true;}
bool psre_m036_get_app(psre_m036_app_id_t id,psre_m036_app_info_t*out){if(id>=2||!apps[id].registered)return false;*out=apps[id];return true;}
bool psre_m032_fill_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c){(void)x;(void)y;(void)w;(void)h;(void)c;++draw_calls;return true;}
bool psre_m032_draw_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c){last_rect_x=x;(void)y;(void)w;(void)h;(void)c;++draw_calls;return true;}
bool psre_m032_draw_line(int32_t x,int32_t y,int32_t a,int32_t b,psre_m032_color_t c){(void)x;(void)y;(void)a;(void)b;(void)c;++draw_calls;return true;}
bool psre_m033_draw_string(const char*s,const psre_m033_text_style_t*st){(void)s;(void)st;++draw_calls;return true;}
void serial_write(const char*s){(void)s;}void serial_write_char(char c){(void)c;}
static void on_click(psre_m038_container_id_t c,psre_m038_widget_id_t w,void*ctx){(void)c;(void)w;(void)ctx;++actions;}
int main(void){
 windows[0]=(psre_m034_window_info_t){.id=0,.x=100,.y=50,.width=420,.height=280,.visible=true};
 windows[1]=(psre_m034_window_info_t){.id=1,.x=300,.y=100,.width=360,.height=240,.visible=true};
 apps[0]=(psre_m036_app_info_t){.registered=true,.lifecycle=PSRE_M036_RUNNING,.window=0};
 apps[1]=(psre_m036_app_info_t){.registered=true,.lifecycle=PSRE_M036_RUNNING,.window=1};
 assert(psre_m038_initialize());
 psre_m038_container_id_t c0=psre_m038_create_container(0),c1=psre_m038_create_container(1);
 assert(c0!=PSRE_M038_INVALID_CONTAINER&&c1!=PSRE_M038_INVALID_CONTAINER&&c0!=c1);
 psre_m038_widget_id_t b0=psre_m038_add_button(c0,"Apply",10,10,100,28,on_click,0);
 psre_m038_widget_id_t t0=psre_m038_add_textbox(c0,"ABC",10,50,140,28,20);
 psre_m038_widget_id_t k0=psre_m038_add_checkbox(c0,"Enabled",10,90,120,true);
 psre_m038_widget_id_t k1=psre_m038_add_checkbox(c1,"Enabled",10,90,120,false);
 assert(b0!=255&&t0!=255&&k0!=255&&k1!=255);
 assert(psre_m038_add_button(c1,"Out of bounds",270,10,100,28,0,0)==PSRE_M038_INVALID_WIDGET);
 assert(psre_m038_get_status()->container_count==2);
 psre_m038_container_info_t ci;assert(psre_m038_get_container(c0,&ci)&&ci.widget_count==3);
 assert(psre_m038_render()&&draw_calls>0);
 wm.focused_window=0;wm.cursor_x=100+2+15;wm.cursor_y=50+24+15;
 psre_input_event_t e={.type=PSRE_INPUT_MOUSE_MOVE};assert(psre_m038_handle_event(&e));
 e=(psre_input_event_t){.type=PSRE_INPUT_MOUSE_BUTTON_DOWN,.code=1,.buttons=1};
 assert(psre_m038_handle_event(&e)&&actions==1);
 assert(psre_m038_focus(c0,t0));
 e=(psre_input_event_t){.type=PSRE_INPUT_KEY_DOWN,.character='Z'};
 assert(psre_m038_handle_event(&e));
 char text[32];assert(psre_m038_get_text(c0,t0,text,sizeof(text))&&!strcmp(text,"ABCZ"));
 e=(psre_input_event_t){.type=PSRE_INPUT_KEY_DOWN,.code=0x2A};assert(psre_m038_handle_event(&e));
 assert(psre_m038_get_text(c0,t0,text,sizeof(text))&&!strcmp(text,"ABC"));
 bool checked=false;assert(psre_m038_is_checked(c0,k0,&checked)&&checked);
 assert(psre_m038_is_checked(c1,k1,&checked)&&!checked);
 wm.focused_window=1;wm.cursor_x=300+2+15;wm.cursor_y=100+24+95;
 e=(psre_input_event_t){.type=PSRE_INPUT_MOUSE_BUTTON_DOWN,.code=1,.buttons=1};
 assert(psre_m038_handle_event(&e));
 assert(psre_m038_is_checked(c1,k1,&checked)&&checked);
 assert(psre_m038_is_checked(c0,k0,&checked)&&checked);
 windows[1].x=340;assert(psre_m038_render());assert(last_rect_x==352);
 windows[1].width=110; /* Existing controls beyond resized client bounds are safely skipped. */
 assert(psre_m038_render());
 apps[0].lifecycle=PSRE_M036_STOPPED;assert(psre_m038_reconcile());
 assert(psre_m038_get_status()->container_count==1);
 puts("M038 window container host tests: PASS");return 0;
}
