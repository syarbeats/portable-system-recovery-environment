#include <assert.h>
#include <stdio.h>
#include <psre/m039/layout.h>
#include <psre/m034/window.h>
#include <psre/m032/graphics.h>
#include <psre/m033/font.h>
static psre_m034_status_t wm;static psre_m034_window_info_t win;static psre_m036_app_info_t app;static unsigned draws;
const psre_m034_status_t *psre_m034_get_status(void){return &wm;}
bool psre_m034_get_window(psre_m034_window_id_t id,psre_m034_window_info_t*out){if(id!=0||!win.visible)return false;*out=win;return true;}
bool psre_m036_get_app(psre_m036_app_id_t id,psre_m036_app_info_t*out){if(id!=0||!app.registered)return false;*out=app;return true;}
bool psre_m032_fill_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c){(void)x;(void)y;(void)w;(void)h;(void)c;++draws;return true;}
bool psre_m032_draw_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c){(void)x;(void)y;(void)w;(void)h;(void)c;++draws;return true;}
bool psre_m032_draw_line(int32_t x,int32_t y,int32_t a,int32_t b,psre_m032_color_t c){(void)x;(void)y;(void)a;(void)b;(void)c;++draws;return true;}
bool psre_m033_draw_string(const char*s,const psre_m033_text_style_t*st){(void)s;(void)st;++draws;return true;}
void serial_write(const char*s){(void)s;}void serial_write_char(char c){(void)c;}
int main(void){
 win=(psre_m034_window_info_t){.id=0,.x=30,.y=40,.width=400,.height=300,.visible=true};
 app=(psre_m036_app_info_t){.registered=true,.lifecycle=PSRE_M036_RUNNING,.window=0};
 assert(psre_m038_initialize());psre_m038_container_id_t c=psre_m038_create_container(0);assert(c!=255);
 psre_m038_widget_id_t a=psre_m038_add_button(c,"One",0,0,80,24,0,0),b=psre_m038_add_button(c,"Two",0,0,80,24,0,0),d=psre_m038_add_checkbox(c,"Three",0,0,100,true);assert(a!=255&&b!=255&&d!=255);
 assert(psre_m039_initialize());psre_m039_theme_t theme={.text={1,2,3},.disabled={4,5,6},.button={7,8,9},.button_hover={10,11,12},.button_pressed={13,14,15},.button_border={16,17,18},.accent={19,20,21},.field_background={22,23,24},.field_text={25,26,27},.check={28,29,30}};assert(psre_m039_set_theme(&theme));assert(psre_m039_apply_vertical(c,8,10,true));
 int32_t x,y,w,h;assert(psre_m038_get_widget_geometry(c,a,&x,&y,&w,&h));assert(x==10&&y==10&&w==376&&h==24);
 assert(psre_m038_get_widget_geometry(c,b,&x,&y,&w,&h));assert(y==42);
 psre_m039_grid_config_t grid={.columns=2,.horizontal_gap=8,.vertical_gap=10,.padding=12,.stretch_width=true};assert(psre_m039_apply_grid(c,&grid));
 assert(psre_m038_get_widget_geometry(c,a,&x,&y,&w,&h));assert(x==12&&y==12&&w>150);
 assert(psre_m038_get_widget_geometry(c,b,&x,&y,&w,&h));assert(x>150&&y==12);
 assert(psre_m039_bring_widget_to_front(c,a));assert(psre_m039_redraw(c)&&draws>0);
 win.height=60;assert(!psre_m039_apply_vertical(c,4,8,true));
 assert(psre_m039_get_status()->layouts_applied>=2);
 puts("M039 layout engine host tests: PASS");return 0;
}
