#include <psre/m037/widget.h>
#include <psre/m030/input.h>
#include <psre/m029/layout.h>
#include <psre/m032/graphics.h>
#include <psre/m033/font.h>
#include <psre/serial.h>
#include <stddef.h>
typedef struct {bool used,enabled,checked,pressed,hovered;psre_m037_widget_type_t type;int32_t x,y,width,height;uint16_t max_length,length;char text[PSRE_M037_TEXT_CAPACITY];psre_m032_color_t foreground;psre_m037_action_t action;void *context;} widget_t;
static widget_t widgets[PSRE_M037_MAX_WIDGETS];static psre_m037_status_t status;
static psre_m032_color_t rgb(uint8_t r,uint8_t g,uint8_t b){psre_m032_color_t c={r,g,b};return c;}
static void copy_text(char*d,uint32_t cap,const char*s){uint32_t i=0;if(!cap)return;while(s&&s[i]&&i+1u<cap){d[i]=s[i];++i;}d[i]=0;}
static bool valid(psre_m037_widget_id_t id){return status.initialized&&id<PSRE_M037_MAX_WIDGETS&&widgets[id].used;}
static psre_m037_widget_id_t alloc(psre_m037_widget_type_t type,const char*t,
                                    int32_t x,int32_t y,int32_t w,int32_t h) {
    if(!status.initialized || !t || w<=0 || h<=0) return PSRE_M037_INVALID_WIDGET;
    for(uint8_t i=0;i<PSRE_M037_MAX_WIDGETS;++i) {
        if(!widgets[i].used) {
            widget_t *p=&widgets[i];
            *p=(widget_t){.used=true,.enabled=true,.type=type,.x=x,.y=y,
                .width=w,.height=h,.max_length=PSRE_M037_TEXT_CAPACITY-1u,
                .foreground={235,242,250}};
            copy_text(p->text,sizeof(p->text),t);
            while(p->text[p->length]) ++p->length;
            ++status.widget_count;
            return i;
        }
    }
    return PSRE_M037_INVALID_WIDGET;
}
bool psre_m037_initialize(void){if(status.initialized)return true;for(uint8_t i=0;i<PSRE_M037_MAX_WIDGETS;++i)widgets[i]=(widget_t){0};status=(psre_m037_status_t){.initialized=true,.focused_widget=PSRE_M037_INVALID_WIDGET};return true;}
void psre_m037_reset(void){for(uint8_t i=0;i<PSRE_M037_MAX_WIDGETS;++i)widgets[i]=(widget_t){0};status=(psre_m037_status_t){.initialized=true,.focused_widget=PSRE_M037_INVALID_WIDGET};}
psre_m037_widget_id_t psre_m037_add_label(const char*t,int32_t x,int32_t y,uint32_t w,psre_m032_color_t fg){psre_m037_widget_id_t id=alloc(PSRE_M037_LABEL,t,x,y,(int32_t)w,16);if(id!=PSRE_M037_INVALID_WIDGET)widgets[id].foreground=fg;return id;}
psre_m037_widget_id_t psre_m037_add_button(const char*t,int32_t x,int32_t y,int32_t w,int32_t h,psre_m037_action_t cb,void*ctx){psre_m037_widget_id_t id=alloc(PSRE_M037_BUTTON,t,x,y,w,h);if(id!=PSRE_M037_INVALID_WIDGET){widgets[id].action=cb;widgets[id].context=ctx;}return id;}
psre_m037_widget_id_t psre_m037_add_textbox(const char*t,int32_t x,int32_t y,int32_t w,int32_t h,uint16_t max){psre_m037_widget_id_t id=alloc(PSRE_M037_TEXTBOX,t,x,y,w,h);if(id!=PSRE_M037_INVALID_WIDGET){if(!max||max>=PSRE_M037_TEXT_CAPACITY)max=PSRE_M037_TEXT_CAPACITY-1u;widgets[id].max_length=max;if(widgets[id].length>max){widgets[id].length=max;widgets[id].text[max]=0;}}return id;}
psre_m037_widget_id_t psre_m037_add_checkbox(const char*t,int32_t x,int32_t y,bool checked){psre_m037_widget_id_t id=alloc(PSRE_M037_CHECKBOX,t,x,y,260,20);if(id!=PSRE_M037_INVALID_WIDGET)widgets[id].checked=checked;return id;}
bool psre_m037_set_text(psre_m037_widget_id_t id,const char*t){if(!valid(id)||!t)return false;copy_text(widgets[id].text,sizeof(widgets[id].text),t);uint32_t n=0;while(widgets[id].text[n])++n;if(widgets[id].type==PSRE_M037_TEXTBOX&&n>widgets[id].max_length){n=widgets[id].max_length;widgets[id].text[n]=0;}widgets[id].length=(uint16_t)n;return true;}
bool psre_m037_get_text(psre_m037_widget_id_t id,char*out,uint32_t cap){if(!valid(id)||!out||!cap)return false;uint32_t i=0;while(widgets[id].text[i]&&i+1u<cap){out[i]=widgets[id].text[i];++i;}out[i]=0;return widgets[id].text[i]==0;}
bool psre_m037_set_checked(psre_m037_widget_id_t id,bool v){if(!valid(id)||widgets[id].type!=PSRE_M037_CHECKBOX)return false;widgets[id].checked=v;return true;}
bool psre_m037_is_checked(psre_m037_widget_id_t id,bool*out){if(!valid(id)||!out||widgets[id].type!=PSRE_M037_CHECKBOX)return false;*out=widgets[id].checked;return true;}
bool psre_m037_set_enabled(psre_m037_widget_id_t id,bool v){if(!valid(id))return false;widgets[id].enabled=v;if(!v&&status.focused_widget==id)status.focused_widget=PSRE_M037_INVALID_WIDGET;return true;}
bool psre_m037_focus(psre_m037_widget_id_t id){if(!valid(id)||!widgets[id].enabled||widgets[id].type==PSRE_M037_LABEL)return false;status.focused_widget=id;return true;}
static bool hit(const widget_t*w,int32_t x,int32_t y){return x>=w->x&&y>=w->y&&x<w->x+w->width&&y<w->y+w->height;}
static psre_m037_widget_id_t next_focus(bool rev){uint8_t start=status.focused_widget<PSRE_M037_MAX_WIDGETS?status.focused_widget:0;for(uint8_t n=1;n<=PSRE_M037_MAX_WIDGETS;++n){uint8_t i=rev?(uint8_t)((start+PSRE_M037_MAX_WIDGETS-n)%PSRE_M037_MAX_WIDGETS):(uint8_t)((start+n)%PSRE_M037_MAX_WIDGETS);if(widgets[i].used&&widgets[i].enabled&&widgets[i].type!=PSRE_M037_LABEL)return i;}return PSRE_M037_INVALID_WIDGET;}
static bool draw_text(const char*s,int32_t x,int32_t y,uint32_t w,uint32_t h,psre_m032_color_t fg){psre_m033_text_style_t st={.x=x,.y=y,.width=w,.height=h,.spacing=1,.font=PSRE_M033_FONT_8X8,.foreground=fg,.background={0,0,0},.opaque_background=false};return psre_m033_draw_string(s,&st);}
bool psre_m037_render(void){if(!status.initialized)return false;bool ok=true;for(uint8_t i=0;i<PSRE_M037_MAX_WIDGETS;++i){widget_t*w=&widgets[i];if(!w->used)continue;bool focus=status.focused_widget==i;psre_m032_color_t fg=w->enabled?w->foreground:rgb(115,125,138);
 if(w->type==PSRE_M037_LABEL)ok=draw_text(w->text,w->x,w->y,(uint32_t)w->width,(uint32_t)w->height,fg)&&ok;
 else if(w->type==PSRE_M037_BUTTON){psre_m032_color_t fill=!w->enabled?rgb(65,72,82):(w->pressed?rgb(25,75,112):(focus?rgb(45,125,170):(w->hovered?rgb(50,115,155):rgb(35,95,135))));ok=psre_m032_fill_rect(w->x,w->y,w->width,w->height,fill)&&ok;ok=psre_m032_draw_rect(w->x,w->y,w->width,w->height,focus?rgb(255,220,90):rgb(190,215,235))&&ok;ok=draw_text(w->text,w->x+6,w->y+(w->height-8)/2,(uint32_t)(w->width-12),12,rgb(255,255,255))&&ok;}
 else if(w->type==PSRE_M037_TEXTBOX){ok=psre_m032_fill_rect(w->x,w->y,w->width,w->height,rgb(245,248,252))&&ok;ok=psre_m032_draw_rect(w->x,w->y,w->width,w->height,focus?rgb(45,145,205):rgb(110,125,140))&&ok;ok=draw_text(w->text,w->x+5,w->y+(w->height-8)/2,(uint32_t)(w->width-10),12,rgb(25,35,48))&&ok;if(focus)ok=psre_m032_fill_rect(w->x+7+(int32_t)w->length*8,w->y+5,1,w->height-10,rgb(30,70,100))&&ok;}
 else if(w->type==PSRE_M037_CHECKBOX){ok=psre_m032_draw_rect(w->x,w->y+2,14,14,focus?rgb(255,220,90):rgb(210,225,238))&&ok;if(w->checked)ok=psre_m032_draw_line(w->x+3,w->y+9,w->x+6,w->y+13,rgb(65,190,125))&&psre_m032_draw_line(w->x+6,w->y+13,w->x+12,w->y+4,rgb(65,190,125))&&ok;ok=draw_text(w->text,w->x+21,w->y+4,(uint32_t)(w->width-21),12,fg)&&ok;}
 }++status.redraws;return ok;}
bool psre_m037_handle_event(const psre_input_event_t*e){if(!status.initialized||!e)return false;++status.events_received;const psre_m030_status_t*in=psre_m030_get_status();
 if(e->type==PSRE_INPUT_MOUSE_MOVE){
    int32_t mx=in?in->mouse_x:0,my=in?in->mouse_y:0;bool changed=false;
    for(uint8_t i=0;i<PSRE_M037_MAX_WIDGETS;++i){widget_t*w=&widgets[i];if(!w->used)continue;bool hover=w->enabled&&w->type!=PSRE_M037_LABEL&&hit(w,mx,my);if(w->hovered!=hover){w->hovered=hover;changed=true;}}
    if(changed){++status.events_handled;(void)psre_m037_render();return true;}
    ++status.rejected_events;return false;
 }
 if(e->type==PSRE_INPUT_MOUSE_BUTTON_DOWN){int32_t x=in?in->mouse_x:0,y=in?in->mouse_y:0;for(int16_t i=(int16_t)PSRE_M037_MAX_WIDGETS-1;i>=0;--i){widget_t*w=&widgets[i];if(!w->used||!w->enabled||!hit(w,x,y))continue;if(w->type==PSRE_M037_LABEL)break;status.focused_widget=(uint8_t)i;if(w->type==PSRE_M037_BUTTON){w->pressed=true;++status.clicks;if(w->action)w->action((uint8_t)i,w->context);}else if(w->type==PSRE_M037_CHECKBOX){w->checked=!w->checked;++status.clicks;}++status.events_handled;(void)psre_m037_render();return true;}}
 if(e->type==PSRE_INPUT_KEY_DOWN||e->type==PSRE_INPUT_KEY_REPEAT){if(e->code==0x2B){status.focused_widget=next_focus((e->modifiers&(PSRE_M029_MOD_LSHIFT|PSRE_M029_MOD_RSHIFT))!=0);++status.events_handled;(void)psre_m037_render();return true;}if(status.focused_widget<PSRE_M037_MAX_WIDGETS){widget_t*w=&widgets[status.focused_widget];if(w->used&&w->enabled){if(e->code==0x28){if(w->type==PSRE_M037_BUTTON){++status.clicks;if(w->action)w->action(status.focused_widget,w->context);}else if(w->type==PSRE_M037_CHECKBOX){w->checked=!w->checked;++status.clicks;}++status.events_handled;(void)psre_m037_render();return true;}if(w->type==PSRE_M037_TEXTBOX){if(e->code==0x2A&&w->length){w->text[--w->length]=0;++status.key_inputs;++status.events_handled;(void)psre_m037_render();return true;}if(e->character>=0x20&&e->character<=0x7E&&w->length<w->max_length){w->text[w->length++]=e->character;w->text[w->length]=0;++status.key_inputs;++status.events_handled;(void)psre_m037_render();return true;}}}}}
 if(e->type==PSRE_INPUT_MOUSE_BUTTON_UP){bool had=false;for(uint8_t i=0;i<PSRE_M037_MAX_WIDGETS;++i)if(widgets[i].used&&widgets[i].pressed){widgets[i].pressed=false;had=true;}if(had){++status.events_handled;(void)psre_m037_render();return true;}}
 ++status.rejected_events;return false;}
const psre_m037_status_t*psre_m037_get_status(void){return &status;}
void psre_m037_report(void){serial_write("[M037] Widgets: ");serial_write(status.initialized?"ready":"not initialized");serial_write(" count=");serial_write_char((char)('0'+status.widget_count/10));serial_write_char((char)('0'+status.widget_count%10));serial_write(" events=");serial_write_char((char)('0'+status.events_handled%10));serial_write("\n");}
