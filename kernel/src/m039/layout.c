#include <psre/m039/layout.h>
#include <psre/m034/window.h>
#include <psre/serial.h>
#include <stddef.h>
static psre_m039_status_t status;
static bool initialized;
static psre_m039_layout_config_t saved[PSRE_M039_MAX_RULES];
static bool saved_valid[PSRE_M039_MAX_RULES];
static bool dimensions(psre_m038_container_id_t id,int32_t *w,int32_t *h){psre_m038_container_info_t ci;psre_m034_window_info_t wi;if(!psre_m038_get_container(id,&ci)||!psre_m034_get_window(ci.window,&wi))return false;*w=wi.width-4;*h=wi.height-26;return *w>0&&*h>0;}
bool psre_m039_set_theme(const psre_m039_theme_t*t){if(!initialized||!t)return false;psre_m038_theme_t m={t->text,t->disabled,t->button,t->button_hover,t->button_pressed,t->button_border,t->accent,t->field_background,t->field_text,t->check};return psre_m038_set_theme(&m);}
bool psre_m039_initialize(void){status=(psre_m039_status_t){0};for(uint8_t i=0;i<PSRE_M039_MAX_RULES;++i)saved_valid[i]=false;initialized=true;return true;}
bool psre_m039_apply_layout(psre_m038_container_id_t id,const psre_m039_layout_config_t *cfg){
 if(!initialized||!cfg||cfg->mode>PSRE_M039_GRID){++status.layout_errors;return false;}
 psre_m038_container_info_t ci;if(!psre_m038_get_container(id,&ci)){++status.layout_errors;return false;}
 int32_t cw,ch;if(!dimensions(id,&cw,&ch))return false;
 psre_m038_widget_id_t ids[PSRE_M039_MAX_RULES];int32_t widths[PSRE_M039_MAX_RULES],heights[PSRE_M039_MAX_RULES];uint8_t n=0;
 for(uint8_t i=0;i<PSRE_M039_MAX_RULES;++i){int32_t x,y,w,h;if(psre_m038_get_widget_geometry(id,i,&x,&y,&w,&h)){ids[n]=i;widths[n]=w;heights[n]=h;++n;}}
 if(!n){++status.layouts_applied;return true;}
 uint16_t pad=cfg->padding,gap=cfg->gap,gap_y=cfg->gap_y;int32_t inner_w=cw-2*(int32_t)pad,inner_h=ch-2*(int32_t)pad;if(inner_w<=0||inner_h<=0)return false;
 uint8_t cols=cfg->mode==PSRE_M039_VERTICAL?1u:(cfg->mode==PSRE_M039_HORIZONTAL?n:(cfg->columns?cfg->columns:1u));if(cols>n)cols=n;
 uint8_t rows=(uint8_t)((n+cols-1u)/cols);int32_t cell_w=(inner_w-(int32_t)(cols-1u)*gap)/(int32_t)cols;
 int32_t y=(int32_t)pad;
 for(uint8_t i=0;i<n;++i){int32_t x=(int32_t)pad,ny=y,w=widths[i],h=heights[i];
  if(cfg->mode==PSRE_M039_VERTICAL){if(cfg->stretch)w=inner_w;ny=y;y+=h+gap_y;}
  else if(cfg->mode==PSRE_M039_HORIZONTAL){x=(int32_t)pad;for(uint8_t j=0;j<i;++j)x+=widths[j]+gap;if(x+w>cw-(int32_t)pad){++status.layout_errors;return false;}}
  else {uint8_t row=(uint8_t)(i/cols),col=(uint8_t)(i%cols);x=(int32_t)pad+(int32_t)col*(cell_w+gap);ny=(int32_t)pad+(int32_t)row*(inner_h/(int32_t)rows)+(int32_t)row*gap_y;if(cfg->stretch)w=cell_w;}
  if(x<0||ny<0||w<=0||h<=0||x+w>cw-(int32_t)pad||ny+h>ch-(int32_t)pad){++status.layout_errors;return false;}
  if(!psre_m038_set_geometry(id,ids[i],x,ny,w,h)){++status.layout_errors;return false;}++status.widgets_positioned;
 }
 saved[id]=*cfg;saved_valid[id]=true;++status.layouts_applied;return true;
}
bool psre_m039_apply_vertical(psre_m038_container_id_t id,uint16_t gap,uint16_t pad,bool stretch){psre_m039_layout_config_t c={PSRE_M039_VERTICAL,1,0,gap,pad,stretch};return psre_m039_apply_layout(id,&c);}
bool psre_m039_apply_horizontal(psre_m038_container_id_t id,uint16_t gap,uint16_t pad){psre_m039_layout_config_t c={PSRE_M039_HORIZONTAL,0,gap,0,pad,false};return psre_m039_apply_layout(id,&c);}
bool psre_m039_apply_grid(psre_m038_container_id_t id,const psre_m039_grid_config_t*g){if(!g)return false;psre_m039_layout_config_t c={PSRE_M039_GRID,g->columns,g->horizontal_gap,g->vertical_gap,g->padding,g->stretch_width};return psre_m039_apply_layout(id,&c);}
bool psre_m039_reflow_all(void){if(!initialized)return false;bool ok=true;for(uint8_t i=0;i<PSRE_M039_MAX_RULES;++i)if(saved_valid[i]){psre_m038_container_info_t ci;if(psre_m038_get_container(i,&ci))ok=psre_m039_apply_layout(i,&saved[i])&&ok;else saved_valid[i]=false;}return ok;}
bool psre_m039_redraw(psre_m038_container_id_t id){if(!initialized)return false;bool ok=psre_m038_render_container(id);if(ok)++status.redraws;return ok;}
bool psre_m039_bring_widget_to_front(psre_m038_container_id_t id,psre_m038_widget_id_t w){return initialized&&psre_m038_bring_to_front(id,w);}
const psre_m039_status_t*psre_m039_get_status(void){return &status;}
void psre_m039_report(void){serial_write("[M039] GUI layout engine ready; layouts=");serial_write_char((char)('0'+status.layouts_applied%10u));serial_write(" positioned=");serial_write_char((char)('0'+status.widgets_positioned%10u));serial_write("\n");}
