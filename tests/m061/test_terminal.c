/* Host test for the GUI terminal: typing, execution via psre_m006_execute with captured
   output, history, builtins, and protection of input-stack commands. */
#include <psre/m061/terminal.h>
#include <psre/m006/fs.h>
#include <psre/m031/framebuffer.h>
#include <psre/m032/graphics.h>
#include <psre/m033/font.h>
#include <psre/m034/window.h>
#include <psre/serial.h>
#include <stdio.h>
#include <string.h>
#include <psre/version.h>

/* ---- stubs ---- */
static serial_sink_t sink; static int sink_installs, stops, renders, presents;
static char executed[16][64]; static int exec_n;
void serial_set_sink(serial_sink_t s){sink=s;if(s)++sink_installs;}
void serial_write_char(char c){if(sink)sink(c);}
void serial_write(const char *t){while(*t){if(*t=='\n')serial_write_char('\r');serial_write_char(*t++);}}
const char *psre_m007_get_cwd(void){return "/DOCS";}
void psre_m006_execute(const char *line){
    snprintf(executed[exec_n++%16],64,"%s",line);
    if(!strcmp(line,"ls"))serial_write("A.TXT\nB.TXT\n");
    else if(!strcmp(line,"info"))serial_write("no newline at end");
    else if(!strcmp(line,"dump")){for(int i=0;i<100;++i){char b[32];snprintf(b,32,"line %03d\n",i);serial_write(b);}}
    else if(!strcmp(line,"help"))serial_write("PSRE recovery shell\n  help  show\n");
    else serial_write("Unknown command. Type 'help'.\n");
}
bool psre_m036_get_app(psre_m036_app_id_t a,psre_m036_app_info_t *o){(void)a;*o=(psre_m036_app_info_t){.registered=true,.lifecycle=PSRE_M036_RUNNING,.window=1};return true;}
bool psre_m036_stop(psre_m036_app_id_t a){(void)a;++stops;return true;}
bool psre_m034_get_window(psre_m034_window_id_t id,psre_m034_window_info_t *o){
    if(id!=1)return false;
    *o=(psre_m034_window_info_t){.id=1,.x=20,.y=20,.width=680,.height=400,.visible=true,.focused=true};
    return true;
}
bool psre_m034_render(void){++renders;return true;}
bool psre_m031_present(void){++presents;return true;}
bool psre_m032_fill_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c){(void)x;(void)y;(void)w;(void)h;(void)c;return true;}
static bool g_large=true;
bool psre_m033_ui_large(void){return g_large;}
uint8_t psre_m033_ui_line_height(void){return g_large?14:8;}
static psre_m034_status_t wm;
const psre_m034_status_t *psre_m034_get_status(void){return &wm;}
bool psre_m032_draw_line(int32_t a,int32_t b,int32_t c,int32_t d,psre_m032_color_t col){(void)a;(void)b;(void)c;(void)d;(void)col;return true;}
bool psre_m033_draw_string(const char *s,const psre_m033_text_style_t *st){(void)s;(void)st;return true;}

#define CHECK(c) do{if(!(c)){printf("FAIL line %d: %s\n",__LINE__,#c);return 1;}}while(0)
static void key(char ch){psre_input_event_t e={.type=PSRE_INPUT_KEY_DOWN,.character=ch};psre_m061_on_event(0,&e,0);}
static void code(uint8_t c){psre_input_event_t e={.type=PSRE_INPUT_KEY_DOWN,.code=c};psre_m061_on_event(0,&e,0);}
static void typeline(const char *s){while(*s)key(*s++);key('\n');}
static bool has(const char *want){char b[160];for(uint32_t i=0;i<200;++i){if(!psre_m061_copy_line(i,b,sizeof b))break;if(!strcmp(b,want))return true;}return false;}
int main(void){
    CHECK(psre_m061_open(0)); CHECK(psre_m061_get_status()->open);
    CHECK(psre_m061_get_status()->cols>=60);
    CHECK(has("PSRE Terminal " PSRE_VERSION_STRING));  /* banner */
    /* run a real command and see its captured output */
    typeline("ls");
    CHECK(!strcmp(executed[0],"ls")); CHECK(has("A.TXT")); CHECK(has("B.TXT")); CHECK(has("PSRE:/DOCS# ls"));
    CHECK(sink==0);                                         /* sink removed afterwards */
    /* output without trailing newline still ends the line */
    typeline("info"); CHECK(has("no newline at end"));
    /* unknown command reaches the CLI (all commands are forwarded) */
    typeline("frobnicate"); CHECK(has("Unknown command. Type 'help'."));
    /* editing: backspace, left, insert, delete */
    key('l');key('x');key('s'); code(0x50);code(0x50);code(0x4C);   /* left,left,delete */
    /* line is now "lxs" with pos=1 and delete removes 'x' => "ls" */
    key('\n'); CHECK(!strcmp(executed[exec_n-1],"ls"));
    key('l');key('s');key('z');key('\b');key('\n'); CHECK(!strcmp(executed[exec_n-1],"ls"));
    /* history: Up recalls last command */
    int before=exec_n; code(0x52); key('\n'); CHECK(exec_n==before+1); CHECK(!strcmp(executed[exec_n-1],"ls"));
    code(0x52);code(0x52); key('\n'); CHECK(exec_n==before+2);
    /* help adds the terminal extras */
    typeline("help"); CHECK(has("Terminal extras:"));
    /* protected commands are blocked, not executed */
    int n0=exec_n; typeline("m030-start");
    CHECK(exec_n==n0); CHECK(psre_m061_get_status()->commands_blocked==1);
    CHECK(has("[TERM] Type 'force m030-start' to run it anyway.")||has("[TERM] Type 'force m030-start' to run it anyway."));
    typeline("M050-PAUSE"); CHECK(exec_n==n0); CHECK(psre_m061_get_status()->commands_blocked==2);
    typeline("m030-poll"); CHECK(exec_n==n0);
    /* force bypasses protection */
    typeline("force m030-start"); CHECK(exec_n==n0+1); CHECK(!strcmp(executed[exec_n-1],"m030-start"));
    typeline("force"); CHECK(exec_n==n0+1); CHECK(has("Usage: force COMMAND"));
    /* cls empties the buffer */
    typeline("cls"); CHECK(!has("A.TXT")); CHECK(!has("PSRE Terminal " PSRE_VERSION_STRING));
    /* flush before running: screen is presented before a command executes */
    int p0=presents; typeline("ls"); CHECK(presents>p0);
    /* ---------------- scroll bar ---------------- */
    typeline("dump");
    psre_m061_scrollbar_t sb;
    CHECK(psre_m061_get_scrollbar(&sb));
    CHECK(sb.total>sb.rows && sb.max_scroll==sb.total-sb.rows && sb.scroll==0);
    CHECK(sb.thumb_y+sb.thumb_h==sb.track_y+sb.track_h);          /* at the bottom */
    CHECK(sb.thumb_h<sb.track_h);
    CHECK(sb.rows==(400-22-4-8)/15);                               /* large font: 15 px rows */
    psre_input_event_t down={.type=PSRE_INPUT_MOUSE_BUTTON_DOWN,.code=1,.buttons=1};
    psre_input_event_t up={.type=PSRE_INPUT_MOUSE_BUTTON_UP,.code=1,.buttons=0};
    psre_input_event_t mv={.type=PSRE_INPUT_MOUSE_MOVE};
    /* up arrow scrolls one line back, down arrow one line forward and stops at 0 */
    wm.cursor_x=sb.x+5; wm.cursor_y=sb.up_y+5;
    psre_m061_on_event(0,&down,0); psre_m061_on_event(0,&up,0);
    psre_m061_on_event(0,&down,0); psre_m061_on_event(0,&up,0);
    CHECK(psre_m061_get_scrollbar(&sb)&&sb.scroll==2);
    CHECK(sb.thumb_y+sb.thumb_h<sb.track_y+sb.track_h);            /* thumb moved up */
    wm.cursor_y=sb.down_y+5;
    for(int i=0;i<5;++i){psre_m061_on_event(0,&down,0);psre_m061_on_event(0,&up,0);}
    CHECK(psre_m061_get_scrollbar(&sb)&&sb.scroll==0);
    /* clicking the track above the thumb pages up, below pages down */
    wm.cursor_y=sb.track_y+3;
    psre_m061_on_event(0,&down,0); psre_m061_on_event(0,&up,0);
    CHECK(psre_m061_get_scrollbar(&sb)&&sb.scroll==sb.rows-1);
    wm.cursor_y=sb.track_y+sb.track_h-3;
    psre_m061_on_event(0,&down,0); psre_m061_on_event(0,&up,0);
    CHECK(psre_m061_get_scrollbar(&sb)&&sb.scroll==0);
    /* dragging the thumb: to the top = oldest line, back to the bottom = newest */
    wm.cursor_y=sb.thumb_y+sb.thumb_h/2;
    psre_m061_on_event(0,&down,0);
    wm.cursor_y=sb.track_y-50; psre_m061_on_event(0,&mv,0);
    CHECK(psre_m061_get_scrollbar(&sb)&&sb.scroll==sb.max_scroll&&sb.thumb_y==sb.track_y);
    wm.cursor_y=sb.track_y+sb.track_h+80; psre_m061_on_event(0,&mv,0);
    CHECK(psre_m061_get_scrollbar(&sb)&&sb.scroll==0);
    wm.cursor_y=sb.track_y+sb.track_h/2; psre_m061_on_event(0,&mv,0);
    CHECK(psre_m061_get_scrollbar(&sb)&&sb.scroll>0&&sb.scroll<sb.max_scroll);
    psre_m061_on_event(0,&up,0);
    uint32_t held=sb.scroll; wm.cursor_y=sb.track_y; psre_m061_on_event(0,&mv,0);   /* released: no more dragging */
    CHECK(psre_m061_get_scrollbar(&sb)&&sb.scroll==held);
    /* clicks outside the bar are ignored by the bar */
    wm.cursor_x=sb.x-40; wm.cursor_y=sb.track_y+3; psre_m061_on_event(0,&down,0); psre_m061_on_event(0,&up,0);
    CHECK(psre_m061_get_scrollbar(&sb)&&sb.scroll==held);
    /* wheel + keys never exceed the scrollable range; typing returns to the bottom */
    psre_input_event_t wheel={.type=PSRE_INPUT_MOUSE_WHEEL,.wheel=1};
    for(int i=0;i<200;++i)psre_m061_on_event(0,&wheel,0);
    CHECK(psre_m061_get_scrollbar(&sb)&&sb.scroll==sb.max_scroll);
    for(int i=0;i<200;++i)code(0x4B);
    CHECK(psre_m061_get_scrollbar(&sb)&&sb.scroll==sb.max_scroll);
    CHECK(psre_m061_get_status()->max_scroll==sb.max_scroll||psre_m061_get_status()->max_scroll==0);
    key('x'); CHECK(psre_m061_get_scrollbar(&sb)&&sb.scroll==0); key('\b');
    /* classic (small) font gives more rows with the same window */
    g_large=false; psre_m061_scrollbar_t sb2; CHECK(psre_m061_get_scrollbar(&sb2));
    CHECK(sb2.rows==(400-22-4-8)/10 && sb2.rows>sb.rows); g_large=true;
    /* a short history has nothing to scroll: thumb fills the track */
    typeline("cls"); CHECK(psre_m061_get_scrollbar(&sb)&&sb.max_scroll==0&&sb.thumb_h==sb.track_h);
    /* halt prints its message before executing */
    typeline("halt"); CHECK(has("System halted."));
    /* scroll keys do not crash and clamp */
    for(int i=0;i<50;++i)code(0x4B);
    for(int i=0;i<100;++i)code(0x4E);
    /* exit closes the app */
    typeline("exit"); CHECK(stops==1); CHECK(!psre_m061_get_status()->open);
    CHECK(psre_m061_render());                    /* closed terminal renders nothing, no crash */
    puts("M061 terminal host test: PASS"); return 0;
}
