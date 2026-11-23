#include <psre/m038/container.h>
#include <psre/m034/window.h>
#include <psre/m032/graphics.h>
#include <psre/m033/font.h>
#include <psre/m029/layout.h>
#include <psre/serial.h>
#include <stddef.h>

#define CONTENT_LEFT 2
#define CONTENT_TOP 24
#define CONTENT_RIGHT 2
#define CONTENT_BOTTOM 2

typedef struct {
 bool used,enabled,checked,pressed,hovered;
 psre_m038_widget_type_t type;
 int32_t x,y,width,height;
 uint16_t length,max_length;
 char text[PSRE_M038_TEXT_CAPACITY];
 psre_m038_action_t action;
 void *context;
} widget_t;
typedef struct {
 bool used;
 psre_m036_app_id_t app;
 psre_m034_window_id_t window;
 uint8_t widget_count;
 psre_m038_widget_id_t focused_widget;
 uint32_t events,clicks,key_inputs,redraws;
 widget_t widgets[PSRE_M038_MAX_WIDGETS];
 psre_m038_widget_id_t z_order[PSRE_M038_MAX_WIDGETS];
} container_t;

static container_t containers[PSRE_M038_MAX_CONTAINERS];
static psre_m038_status_t status;
static psre_m038_theme_t theme={
 .text={35,43,55},.disabled={130,135,145},.button={35,95,135},
 .button_hover={50,115,155},.button_pressed={25,75,112},.button_border={85,105,125},
 .accent={45,125,170},.field_background={250,251,253},.field_text={25,35,48},.check={35,155,95}
};

static psre_m032_color_t rgb(uint8_t r,uint8_t g,uint8_t b){psre_m032_color_t c={r,g,b};return c;}
static void copy_text(char *dst,uint32_t cap,const char *src){uint32_t i=0;if(!cap)return;while(src&&src[i]&&i+1u<cap){dst[i]=src[i];++i;}dst[i]=0;}
static container_t *get_container(psre_m038_container_id_t id){return status.initialized&&id<PSRE_M038_MAX_CONTAINERS&&containers[id].used?&containers[id]:0;}
static widget_t *get_widget(container_t *c,psre_m038_widget_id_t id){return c&&id<PSRE_M038_MAX_WIDGETS&&c->widgets[id].used?&c->widgets[id]:0;}
static bool geometry(container_t *c,psre_m034_window_info_t *wi,int32_t *ox,int32_t *oy,int32_t *cw,int32_t *ch){
 if(!c||!psre_m034_get_window(c->window,wi)||!wi->visible)return false;
 *ox=wi->x+CONTENT_LEFT;*oy=wi->y+CONTENT_TOP;
 *cw=wi->width-CONTENT_LEFT-CONTENT_RIGHT;
 *ch=wi->height-CONTENT_TOP-CONTENT_BOTTOM;
 return *cw>0&&*ch>0;
}
static bool bounds_ok(container_t *c,int32_t x,int32_t y,int32_t w,int32_t h){
 psre_m034_window_info_t wi;int32_t ox,oy,cw,ch;(void)ox;(void)oy;
 return w>0&&h>0&&geometry(c,&wi,&ox,&oy,&cw,&ch)&&x>=0&&y>=0&&
        x<=cw-w&&y<=ch-h;
}
static psre_m038_widget_id_t allocate(container_t *c,psre_m038_widget_type_t type,
                                       const char *text,int32_t x,int32_t y,int32_t w,int32_t h){
 if(!c||!text)return PSRE_M038_INVALID_WIDGET;
 if((type==PSRE_M038_BUTTON||type==PSRE_M038_TEXTBOX)&&(w<24||h<16))return PSRE_M038_INVALID_WIDGET;
 if(type==PSRE_M038_CHECKBOX&&(w<28||h<20))return PSRE_M038_INVALID_WIDGET;
 if(!bounds_ok(c,x,y,w,h))return PSRE_M038_INVALID_WIDGET;
 for(uint8_t i=0;i<PSRE_M038_MAX_WIDGETS;++i)if(!c->widgets[i].used){
  widget_t *v=&c->widgets[i];*v=(widget_t){.used=true,.enabled=true,.type=type,
   .x=x,.y=y,.width=w,.height=h,.max_length=PSRE_M038_TEXT_CAPACITY-1u};
  copy_text(v->text,sizeof(v->text),text);while(v->text[v->length])++v->length;
  if(type==PSRE_M038_TEXTBOX&&v->length>v->max_length){v->length=v->max_length;v->text[v->length]=0;}
  c->z_order[c->widget_count]=i;++c->widget_count;return i;
 }
 return PSRE_M038_INVALID_WIDGET;
}
bool psre_m038_set_theme(const psre_m038_theme_t*t){if(!t)return false;theme=*t;return true;}
bool psre_m038_initialize(void){
 if(status.initialized)return true;
 for(uint8_t i=0;i<PSRE_M038_MAX_CONTAINERS;++i)containers[i]=(container_t){0};
 status=(psre_m038_status_t){.initialized=true};return true;
}
void psre_m038_reset(void){for(uint8_t i=0;i<PSRE_M038_MAX_CONTAINERS;++i)containers[i]=(container_t){0};status=(psre_m038_status_t){.initialized=true};}
psre_m038_container_id_t psre_m038_create_container(psre_m036_app_id_t app){
 if(!status.initialized)return PSRE_M038_INVALID_CONTAINER;
 psre_m036_app_info_t ai;if(!psre_m036_get_app(app,&ai)||!ai.registered||ai.lifecycle!=PSRE_M036_RUNNING||ai.window==PSRE_M034_INVALID_WINDOW)return PSRE_M038_INVALID_CONTAINER;
 psre_m034_window_info_t wi;if(!psre_m034_get_window(ai.window,&wi))return PSRE_M038_INVALID_CONTAINER;
 for(uint8_t i=0;i<PSRE_M038_MAX_CONTAINERS;++i)if(containers[i].used&&containers[i].app==app&&containers[i].window==ai.window)return i;
 for(uint8_t i=0;i<PSRE_M038_MAX_CONTAINERS;++i)if(!containers[i].used){
  containers[i]=(container_t){.used=true,.app=app,.window=ai.window,.focused_widget=PSRE_M038_INVALID_WIDGET};
  ++status.container_count;return i;
 }
 return PSRE_M038_INVALID_CONTAINER;
}
bool psre_m038_destroy_container(psre_m038_container_id_t id){
 container_t*c=get_container(id);if(!c)return false;*c=(container_t){0};
 if(status.container_count) --status.container_count;
 return true;
}
bool psre_m038_reconcile(void){
 if(!status.initialized)return false;
 for(uint8_t i=0;i<PSRE_M038_MAX_CONTAINERS;++i)if(containers[i].used){
  psre_m036_app_info_t ai;
  if(!psre_m036_get_app(containers[i].app,&ai)||ai.lifecycle!=PSRE_M036_RUNNING||
     ai.window!=containers[i].window)(void)psre_m038_destroy_container(i);
 }
 return true;
}
bool psre_m038_get_container(psre_m038_container_id_t id,psre_m038_container_info_t*out){container_t*c=get_container(id);if(!c||!out)return false;*out=(psre_m038_container_info_t){.used=c->used,.app=c->app,.window=c->window,.widget_count=c->widget_count,.focused_widget=c->focused_widget,.events=c->events,.clicks=c->clicks,.key_inputs=c->key_inputs,.redraws=c->redraws};return true;}
psre_m038_widget_id_t psre_m038_add_label(psre_m038_container_id_t id,const char*t,int32_t x,int32_t y,uint32_t w){return allocate(get_container(id),PSRE_M038_LABEL,t,x,y,(int32_t)w,16);}
psre_m038_widget_id_t psre_m038_add_button(psre_m038_container_id_t id,const char*t,int32_t x,int32_t y,int32_t w,int32_t h,psre_m038_action_t cb,void*ctx){container_t*c=get_container(id);psre_m038_widget_id_t v=allocate(c,PSRE_M038_BUTTON,t,x,y,w,h);if(v!=PSRE_M038_INVALID_WIDGET){c->widgets[v].action=cb;c->widgets[v].context=ctx;}return v;}
psre_m038_widget_id_t psre_m038_add_textbox(psre_m038_container_id_t id,const char*t,int32_t x,int32_t y,int32_t w,int32_t h,uint16_t max){container_t*c=get_container(id);psre_m038_widget_id_t v=allocate(c,PSRE_M038_TEXTBOX,t,x,y,w,h);if(v!=PSRE_M038_INVALID_WIDGET){if(!max||max>=PSRE_M038_TEXT_CAPACITY)max=PSRE_M038_TEXT_CAPACITY-1u;c->widgets[v].max_length=max;if(c->widgets[v].length>max){c->widgets[v].length=max;c->widgets[v].text[max]=0;}}return v;}
psre_m038_widget_id_t psre_m038_add_checkbox(psre_m038_container_id_t id,const char*t,int32_t x,int32_t y,uint32_t w,bool checked){container_t*c=get_container(id);psre_m038_widget_id_t v=allocate(c,PSRE_M038_CHECKBOX,t,x,y,(int32_t)w,20);if(v!=PSRE_M038_INVALID_WIDGET)c->widgets[v].checked=checked;return v;}
bool psre_m038_set_text(psre_m038_container_id_t id,psre_m038_widget_id_t wid,const char*t){widget_t*w=get_widget(get_container(id),wid);if(!w||!t)return false;copy_text(w->text,sizeof(w->text),t);uint32_t n=0;while(w->text[n])++n;if(w->type==PSRE_M038_TEXTBOX&&n>w->max_length){n=w->max_length;w->text[n]=0;}w->length=(uint16_t)n;return true;}
bool psre_m038_get_text(psre_m038_container_id_t id,psre_m038_widget_id_t wid,char*out,uint32_t cap){widget_t*w=get_widget(get_container(id),wid);if(!w||!out||!cap)return false;uint32_t i=0;while(w->text[i]&&i+1u<cap){out[i]=w->text[i];++i;}out[i]=0;return w->text[i]==0;}
bool psre_m038_set_checked(psre_m038_container_id_t id,psre_m038_widget_id_t wid,bool v){widget_t*w=get_widget(get_container(id),wid);if(!w||w->type!=PSRE_M038_CHECKBOX)return false;w->checked=v;return true;}
bool psre_m038_is_checked(psre_m038_container_id_t id,psre_m038_widget_id_t wid,bool*out){widget_t*w=get_widget(get_container(id),wid);if(!w||!out||w->type!=PSRE_M038_CHECKBOX)return false;*out=w->checked;return true;}
bool psre_m038_set_enabled(psre_m038_container_id_t id,psre_m038_widget_id_t wid,bool v){container_t*c=get_container(id);widget_t*w=get_widget(c,wid);if(!w)return false;w->enabled=v;if(!v&&c->focused_widget==wid)c->focused_widget=PSRE_M038_INVALID_WIDGET;return true;}
bool psre_m038_get_widget_geometry(psre_m038_container_id_t id,psre_m038_widget_id_t wid,int32_t*x,int32_t*y,int32_t*w,int32_t*h){widget_t*v=get_widget(get_container(id),wid);if(!v||!x||!y||!w||!h)return false;*x=v->x;*y=v->y;*w=v->width;*h=v->height;return true;}
bool psre_m038_set_geometry(psre_m038_container_id_t id,psre_m038_widget_id_t wid,int32_t x,int32_t y,int32_t w,int32_t h){container_t*c=get_container(id);widget_t*v=get_widget(c,wid);if(!v||!bounds_ok(c,x,y,w,h))return false;v->x=x;v->y=y;v->width=w;v->height=h;return true;}
bool psre_m038_bring_to_front(psre_m038_container_id_t id,psre_m038_widget_id_t wid){container_t*c=get_container(id);if(!get_widget(c,wid))return false;uint8_t pos=0;while(pos<c->widget_count&&c->z_order[pos]!=wid)++pos;if(pos>=c->widget_count)return false;for(uint8_t i=pos;i+1u<c->widget_count;++i)c->z_order[i]=c->z_order[i+1u];c->z_order[c->widget_count-1u]=wid;return true;}
bool psre_m038_focus(psre_m038_container_id_t id,psre_m038_widget_id_t wid){container_t*c=get_container(id);widget_t*w=get_widget(c,wid);if(!w||!w->enabled||w->type==PSRE_M038_LABEL)return false;c->focused_widget=wid;return true;}
static bool hit(widget_t*w,int32_t x,int32_t y){return x>=w->x&&y>=w->y&&x<w->x+w->width&&y<w->y+w->height;}
static psre_m038_widget_id_t next_focus(container_t*c,bool reverse){uint8_t start=c->focused_widget<PSRE_M038_MAX_WIDGETS?c->focused_widget:0;for(uint8_t n=1;n<=PSRE_M038_MAX_WIDGETS;++n){uint8_t i=reverse?(uint8_t)((start+PSRE_M038_MAX_WIDGETS-n)%PSRE_M038_MAX_WIDGETS):(uint8_t)((start+n)%PSRE_M038_MAX_WIDGETS);widget_t*w=&c->widgets[i];if(w->used&&w->enabled&&w->type!=PSRE_M038_LABEL)return i;}return PSRE_M038_INVALID_WIDGET;}
bool psre_m038_handle_event(const psre_input_event_t*e){
 if(!status.initialized||!e)return false;
 ++status.events_received;
 const psre_m034_status_t*wm=psre_m034_get_status();if(!wm){++status.events_ignored;return false;}
 container_t*c=0;for(uint8_t i=0;i<PSRE_M038_MAX_CONTAINERS;++i)if(containers[i].used&&containers[i].window==wm->focused_window){c=&containers[i];break;}
 if(!c){++status.events_ignored;return false;}psre_m034_window_info_t wi;int32_t ox,oy,cw,ch;
 if(!geometry(c,&wi,&ox,&oy,&cw,&ch)){++status.events_ignored;return false;}
 ++c->events;
 if(e->type==PSRE_INPUT_MOUSE_MOVE){
  int32_t x=wm->cursor_x-ox,y=wm->cursor_y-oy;bool changed=false;
  for(uint8_t i=0;i<PSRE_M038_MAX_WIDGETS;++i){widget_t*w=&c->widgets[i];if(!w->used)continue;bool hover=w->enabled&&w->type!=PSRE_M038_LABEL&&hit(w,x,y);if(w->hovered!=hover){w->hovered=hover;changed=true;}}
  if(changed){++status.events_handled;(void)psre_m038_render();return true;}++status.events_ignored;return false;
 }
 if(e->type==PSRE_INPUT_MOUSE_BUTTON_DOWN&&(e->code==1u||(e->buttons&1u))){
  int32_t x=wm->cursor_x-ox,y=wm->cursor_y-oy;
  for(int16_t pos=(int16_t)c->widget_count-1;pos>=0;--pos){uint8_t i=c->z_order[pos];widget_t*w=&c->widgets[i];if(!w->used||!w->enabled||!hit(w,x,y))continue;if(w->type==PSRE_M038_LABEL)break;c->focused_widget=(uint8_t)i;if(w->type==PSRE_M038_BUTTON){w->pressed=true;++c->clicks;if(w->action)w->action((uint8_t)(c-containers),(uint8_t)i,w->context);}else if(w->type==PSRE_M038_CHECKBOX){w->checked=!w->checked;++c->clicks;}++status.events_handled;(void)psre_m038_render();return true;}
 }
 if(e->type==PSRE_INPUT_MOUSE_BUTTON_UP&&(e->code==1u||!(e->buttons&1u))){bool had=false;for(uint8_t i=0;i<PSRE_M038_MAX_WIDGETS;++i)if(c->widgets[i].used&&c->widgets[i].pressed){c->widgets[i].pressed=false;had=true;}if(had){++status.events_handled;(void)psre_m038_render();return true;}}
 if(e->type==PSRE_INPUT_KEY_DOWN||e->type==PSRE_INPUT_KEY_REPEAT){
  if(e->code==0x2Bu){c->focused_widget=next_focus(c,(e->modifiers&(PSRE_M029_MOD_LSHIFT|PSRE_M029_MOD_RSHIFT))!=0);++status.events_handled;(void)psre_m038_render();return true;}
  widget_t*w=get_widget(c,c->focused_widget);
  if(w&&w->enabled){
   if(e->code==0x28u){if(w->type==PSRE_M038_BUTTON){++c->clicks;if(w->action)w->action((uint8_t)(c-containers),c->focused_widget,w->context);}else if(w->type==PSRE_M038_CHECKBOX){w->checked=!w->checked;++c->clicks;}else{++status.events_ignored;return false;}++status.events_handled;(void)psre_m038_render();return true;}
   if(w->type==PSRE_M038_TEXTBOX){if(e->code==0x2Au&&w->length){w->text[--w->length]=0;++c->key_inputs;++status.events_handled;(void)psre_m038_render();return true;}if(e->character>=0x20&&e->character<=0x7E&&w->length<w->max_length){w->text[w->length++]=e->character;w->text[w->length]=0;++c->key_inputs;++status.events_handled;(void)psre_m038_render();return true;}}
  }
 }
 ++status.events_ignored;return false;
}
static bool draw_text(const char*s,int32_t x,int32_t y,uint32_t w,uint32_t h,psre_m032_color_t fg){psre_m033_text_style_t st={.x=x,.y=y,.width=w,.height=h,.spacing=1,.font=PSRE_M033_FONT_8X8,.foreground=fg,.background={0,0,0},.opaque_background=false};return psre_m033_draw_string(s,&st);}
static bool render_container(uint8_t cid,container_t*c){
 psre_m034_window_info_t wi;int32_t ox,oy,cw,ch;if(!geometry(c,&wi,&ox,&oy,&cw,&ch))return false;
 bool ok=true;
 for(uint8_t pos=0;pos<c->widget_count;++pos){uint8_t i=c->z_order[pos];widget_t*w=&c->widgets[i];if(!w->used)continue;
  if(!bounds_ok(c,w->x,w->y,w->width,w->height)){++status.render_errors;continue;}
  int32_t x=ox+w->x,y=oy+w->y;bool focused=c->focused_widget==i;
  psre_m032_color_t fg=w->enabled?theme.text:theme.disabled;
  if(w->type==PSRE_M038_LABEL)ok=draw_text(w->text,x,y,(uint32_t)w->width,(uint32_t)w->height,fg)&&ok;
  else if(w->type==PSRE_M038_BUTTON){psre_m032_color_t fill=!w->enabled?rgb(115,120,128):(w->pressed?theme.button_pressed:(focused?theme.accent:(w->hovered?theme.button_hover:theme.button)));ok=psre_m032_fill_rect(x,y,w->width,w->height,fill)&&ok;ok=psre_m032_draw_rect(x,y,w->width,w->height,focused?theme.accent:theme.button_border)&&ok;ok=draw_text(w->text,x+6,y+(w->height-8)/2,(uint32_t)(w->width-12),12,theme.field_background)&&ok;}
  else if(w->type==PSRE_M038_TEXTBOX){ok=psre_m032_fill_rect(x,y,w->width,w->height,theme.field_background)&&ok;ok=psre_m032_draw_rect(x,y,w->width,w->height,focused?rgb(35,125,190):rgb(115,125,140))&&ok;ok=draw_text(w->text,x+5,y+(w->height-8)/2,(uint32_t)(w->width-10),12,theme.field_text)&&ok;if(focused){uint32_t visible_chars=(uint32_t)(w->width-12)/8u;uint32_t cursor_chars=w->length<visible_chars?w->length:visible_chars;ok=psre_m032_fill_rect(x+7+(int32_t)cursor_chars*8,y+5,1,w->height-10,rgb(30,70,100))&&ok;}}
  else if(w->type==PSRE_M038_CHECKBOX){ok=psre_m032_draw_rect(x,y+2,14,14,focused?theme.accent:theme.button_border)&&ok;if(w->checked)ok=psre_m032_draw_line(x+3,y+9,x+6,y+13,theme.check)&&psre_m032_draw_line(x+6,y+13,x+12,y+4,theme.check)&&ok;ok=draw_text(w->text,x+21,y+4,(uint32_t)(w->width-21),12,fg)&&ok;}
 }
 ++c->redraws;(void)cid;(void)cw;(void)ch;return ok;
}
bool psre_m038_render_container(psre_m038_container_id_t id){container_t*c=get_container(id);if(!c)return false;++status.render_calls;bool ok=render_container(id,c);if(!ok)++status.render_errors;return ok;}
bool psre_m038_render(void){
 if(!status.initialized)return false;
 (void)psre_m038_reconcile();
 ++status.render_calls;bool ok=true;
 /* Render background containers first and focused window last. */
 for(uint8_t pass=0;pass<2;++pass)for(uint8_t i=0;i<PSRE_M038_MAX_CONTAINERS;++i){container_t*c=&containers[i];if(!c->used)continue;const psre_m034_status_t*wm=psre_m034_get_status();bool focused=wm&&wm->focused_window==c->window;if((pass==0&&focused)||(pass==1&&!focused))continue;if(!render_container(i,c)){++status.render_errors;ok=false;}}
 return ok;
}
const psre_m038_status_t*psre_m038_get_status(void){return &status;}
void psre_m038_report(void){serial_write("[M038] Window containers: ");serial_write(status.initialized?"ready":"not initialized");serial_write(" count=");serial_write_char((char)('0'+status.container_count));serial_write(" events=");serial_write_char((char)('0'+status.events_handled%10u));serial_write("\n");}
