#include <assert.h>
#include <stdio.h>
#include <psre/m040/controls.h>
#include <psre/m034/window.h>
#include <psre/m032/graphics.h>
#include <psre/m033/font.h>
static psre_m034_window_info_t win;static psre_m036_app_info_t app;static psre_m030_status_t input;static unsigned draws;
const psre_m030_status_t *psre_m030_get_status(void){return &input;}
const psre_m034_status_t *psre_m034_get_status(void){static psre_m034_status_t s;return &s;}
bool psre_m034_get_window(psre_m034_window_id_t id,psre_m034_window_info_t*out){if(id||!win.visible)return false;*out=win;return true;}
bool psre_m036_get_app(psre_m036_app_id_t id,psre_m036_app_info_t*out){if(id||!app.registered)return false;*out=app;return true;}
bool psre_m038_get_container(psre_m038_container_id_t id,psre_m038_container_info_t*out){if(id)return false;*out=(psre_m038_container_info_t){.used=true,.app=0,.window=0};return true;}
bool psre_m032_fill_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c){(void)x;(void)y;(void)w;(void)h;(void)c;++draws;return true;}
bool psre_m032_draw_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c){(void)x;(void)y;(void)w;(void)h;(void)c;++draws;return true;}
bool psre_m032_fill_circle(int32_t x,int32_t y,int32_t r,psre_m032_color_t c){(void)x;(void)y;(void)r;(void)c;++draws;return true;}
bool psre_m033_draw_string(const char*s,const psre_m033_text_style_t*st){(void)s;(void)st;++draws;return true;}
void serial_write(const char*s){(void)s;}void serial_write_char(char c){(void)c;}
int main(void){win=(psre_m034_window_info_t){.id=0,.x=10,.y=10,.width=420,.height=300,.visible=true};app=(psre_m036_app_info_t){.registered=true,.lifecycle=PSRE_M036_RUNNING,.window=0};assert(psre_m040_initialize());
 psre_m040_control_id_t combo=psre_m040_create(0,PSRE_M040_COMBO,"Device",10,10,160,48);assert(combo!=255);assert(psre_m040_add_item(combo,"USB Keyboard"));assert(psre_m040_add_item(combo,"USB Mouse"));assert(psre_m040_set_value(combo,1));int32_t v=-1;assert(psre_m040_get_value(combo,&v)&&v==1);
 psre_m040_control_id_t list=psre_m040_create(0,PSRE_M040_LIST,"Partitions",10,70,180,90);assert(list!=255);assert(psre_m040_add_item(list,"EFI System"));assert(psre_m040_add_item(list,"Recovery"));assert(psre_m040_set_value(list,0));
 psre_m040_control_id_t table=psre_m040_create(0,PSRE_M040_TABLE,"Disk",200,10,190,90);assert(table!=255);assert(psre_m040_set_table_cell(table,0,0,"Disk 0"));assert(psre_m040_set_table_cell(table,0,1,"GPT"));
 input.mouse_x=win.x+2+210;input.mouse_y=win.y+24+10+22;psre_input_event_t table_click={.type=PSRE_INPUT_MOUSE_BUTTON_DOWN};assert(psre_m040_handle_event(&table_click));uint8_t selected=99;assert(psre_m040_get_table_selection(table,&selected)&&selected==0);assert(!psre_m040_get_table_selection(table,&selected));
 psre_m040_control_id_t slider=psre_m040_create(0,PSRE_M040_SLIDER,"Brightness",10,180,180,42);assert(slider!=255);assert(psre_m040_set_range(slider,0,10));assert(psre_m040_set_value(slider,7));assert(psre_m040_get_value(slider,&v)&&v==7);
 psre_m040_control_id_t progress=psre_m040_create(0,PSRE_M040_PROGRESS,"Recovery progress",200,120,180,48);assert(progress!=255);assert(psre_m040_set_value(progress,45));assert(psre_m040_render_container(0)&&draws>0);
 input.mouse_x=win.x+2+10+30;input.mouse_y=win.y+24+180+20;psre_input_event_t click={.type=PSRE_INPUT_MOUSE_BUTTON_DOWN};assert(psre_m040_handle_event(&click));assert(psre_m040_get_value(slider,&v)&&v>=0&&v<=10);
 assert(psre_m040_set_modal(true,"Confirm","Continue recovery?"));assert(psre_m040_render());psre_input_event_t e={.type=PSRE_INPUT_KEY_DOWN,.code=0x28};assert(psre_m040_handle_event(&e));assert(!psre_m040_get_status()->modal_visible);
 assert(psre_m040_get_status()->control_count==5);puts("M040 advanced GUI controls host tests: PASS");return 0;}
