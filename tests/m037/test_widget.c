#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <psre/m037/widget.h>
#include <psre/m033/font.h>
#include <psre/m029/layout.h>
static psre_m030_status_t in;static unsigned fills,rects,lines,texts,actions;
const psre_m030_status_t*psre_m030_get_status(void){return &in;}
bool psre_m032_fill_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c){(void)x;(void)y;(void)w;(void)h;(void)c;++fills;return true;}
bool psre_m032_draw_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c){(void)x;(void)y;(void)w;(void)h;(void)c;++rects;return true;}
bool psre_m032_draw_line(int32_t x,int32_t y,int32_t a,int32_t b,psre_m032_color_t c){(void)x;(void)y;(void)a;(void)b;(void)c;++lines;return true;}
bool psre_m033_draw_string(const char*s,const psre_m033_text_style_t*st){(void)s;(void)st;++texts;return true;}
void serial_write(const char*s){(void)s;}void serial_write_char(char c){(void)c;}
static void action(psre_m037_widget_id_t id,void*ctx){(void)id;(void)ctx;++actions;}
int main(void){assert(psre_m037_initialize());psre_m037_widget_id_t l=psre_m037_add_label("Name",10,10,100,(psre_m032_color_t){255,255,255});psre_m037_widget_id_t b=psre_m037_add_button("Apply",10,40,100,28,action,0);psre_m037_widget_id_t t=psre_m037_add_textbox("abc",10,80,160,28,12);psre_m037_widget_id_t c=psre_m037_add_checkbox("Enable",10,120,false);assert(l!=255&&b!=255&&t!=255&&c!=255);assert(psre_m037_get_status()->widget_count==4);assert(psre_m037_render()&&fills&&rects&&texts);assert(psre_m037_focus(b));psre_input_event_t e={.type=PSRE_INPUT_KEY_DOWN,.code=0x28};assert(psre_m037_handle_event(&e)&&actions==1);in.mouse_x=20;in.mouse_y=125;e=(psre_input_event_t){.type=PSRE_INPUT_MOUSE_MOVE};assert(psre_m037_handle_event(&e));e=(psre_input_event_t){.type=PSRE_INPUT_MOUSE_BUTTON_DOWN};assert(psre_m037_handle_event(&e));bool checked=false;assert(psre_m037_is_checked(c,&checked)&&checked);assert(psre_m037_set_checked(c,false));assert(psre_m037_focus(t));e=(psre_input_event_t){.type=PSRE_INPUT_KEY_DOWN,.character='Z'};assert(psre_m037_handle_event(&e));char value[32];assert(psre_m037_get_text(t,value,sizeof(value))&&!strcmp(value,"abcZ"));e=(psre_input_event_t){.type=PSRE_INPUT_KEY_DOWN,.code=0x2A};assert(psre_m037_handle_event(&e));assert(psre_m037_get_text(t,value,sizeof(value))&&!strcmp(value,"abc"));e=(psre_input_event_t){.type=PSRE_INPUT_KEY_DOWN,.code=0x2B};assert(psre_m037_handle_event(&e));e.modifiers=PSRE_M029_MOD_LSHIFT;assert(psre_m037_handle_event(&e));assert(psre_m037_set_enabled(b,false)&&!psre_m037_focus(b));assert(psre_m037_set_text(l,"Updated"));assert(psre_m037_get_status()->events_handled>=5);puts("M037 interactive widget host tests: PASS");return 0;}
