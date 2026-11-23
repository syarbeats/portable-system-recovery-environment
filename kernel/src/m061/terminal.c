#include <psre/m061/terminal.h>
#include <psre/m006/fs.h>
#include <psre/m031/framebuffer.h>
#include <psre/m032/graphics.h>
#include <psre/m033/font.h>
#include <psre/m034/window.h>
#include <psre/serial.h>
#include <psre/version.h>

#define CELL_W 9      /* 8x8 font + 1px spacing */
#define SB_W 14       /* scroll bar width */
#define WIN_BORDER 2  /* must match M034 */
#define WIN_TITLE 22
#define PAD_X 6
#define PAD_Y 4
#define LINES PSRE_M061_MAX_LINES
#define COLS PSRE_M061_MAX_COLS
#define INPUT_MAX PSRE_M061_INPUT_MAX
#define HIST PSRE_M061_HISTORY
#define MOD_CTRL 0x11u
#define MOD_ALT 0x44u

typedef struct {
    bool open, busy;
    psre_m036_app_id_t app;
    uint16_t cols, rows;
    char lines[LINES][COLS + 1u];
    uint8_t kind[LINES];            /* 0 output, 1 echoed command */
    uint32_t count;                 /* index of the line currently written to */
    uint16_t col;
    uint32_t scroll;                /* lines scrolled back from the bottom */
    bool sb_drag; int32_t sb_drag_off;
    char input[INPUT_MAX];
    uint16_t len, pos;
    char history[HIST][INPUT_MAX];
    uint8_t hist_count, hist_cursor;
    psre_m061_status_t status;
} term_t;
static term_t T;
typedef struct {
    int32_t bx, by, bw, bh, tx, ty;
    uint32_t cols, rows, ch;
    psre_m061_scrollbar_t sb;
} geom_t;
static bool compute_geometry(geom_t *g,bool *focused);
static bool scrollbar_mouse(const psre_input_event_t *e);

static void zero_state(void){uint8_t *p=(uint8_t *)&T;for(uint32_t i=0;i<sizeof(T);++i)p[i]=0;}
static psre_m032_color_t rgb(uint8_t r,uint8_t g,uint8_t b){psre_m032_color_t c={r,g,b};return c;}

/* ---- output buffer ------------------------------------------------------ */
static char *cur_line(void){return T.lines[T.count%LINES];}
static void new_line(void){
    ++T.count; ++T.status.lines_written;
    T.lines[T.count%LINES][0]=0; T.kind[T.count%LINES]=0; T.col=0;
}
static void put_char(char c){
    if(c=='\r')return;
    if(c=='\n'){new_line();return;}
    if(c=='\t'){do{ if(T.col>=T.cols)new_line(); cur_line()[T.col++]=' '; cur_line()[T.col]=0; }while(T.col%4u);return;}
    if((unsigned char)c<0x20||(unsigned char)c>0x7e)return;
    if(T.col>=T.cols)new_line();
    cur_line()[T.col++]=c; cur_line()[T.col]=0;
}
static void sink(char c){put_char(c);}
static void puts_term(const char *s){while(*s)put_char(*s++);}
static void ensure_newline(void){if(T.col)new_line();}
static void clear_screen(void){
    for(uint32_t i=0;i<LINES;++i){T.lines[i][0]=0;T.kind[i]=0;}
    T.count=0;T.col=0;T.scroll=0;
}

/* ---- small helpers ------------------------------------------------------ */
static char lower(char c){return (c>='A'&&c<='Z')?(char)(c+32):c;}
static bool word_is(const char *w,const char *lit){
    while(*w&&*lit){if(lower(*w)!=*lit)return false;++w;++lit;}
    return !*w&&!*lit;
}
static void make_prompt(char out[64]){
    const char *cwd=psre_m007_get_cwd();
    uint32_t n=0; const char *pre="PSRE:";
    while(*pre&&n<60u)out[n++]=*pre++;
    uint32_t l=0; if(cwd)while(cwd[l])++l;
    uint32_t start=l>30u?l-30u:0u;
    for(uint32_t i=start;i<l&&n<60u;++i)out[n++]=cwd[i];
    out[n++]='#'; out[n++]=' '; out[n]=0;
}
static void flush_screen(void){
    ++T.status.renders;
    (void)psre_m034_render();
    (void)psre_m031_present();
}

/* Commands that would break the running desktop (input stack) if executed from inside it. */
static const char *protected_reason(const char *w){
    static const char *const reinit[]={"m015-init","xhci-release","m016-reset","m017-enable-slot",
        "m018-enumerate","m020-configure","m020-alt","m025-start","m026-start","m030-start",0};
    static const char *const polls[]={"m030-poll","m034-poll","m035-poll","m049-poll",0};
    for(uint8_t i=0;reinit[i];++i)if(word_is(w,reinit[i]))return "it re-initializes the USB/input stack this terminal is using";
    for(uint8_t i=0;polls[i];++i)if(word_is(w,polls[i]))return "it would re-enter the input dispatcher (the desktop already polls input)";
    if(word_is(w,"m050-pause"))return "it pauses the event loop that delivers keyboard and mouse input";
    return 0;
}

static void extra_help(void){
    puts_term("\nTerminal extras:\n"
              "  cls            clear this window (the CLI 'clear' frees the RAM workspace)\n"
              "  exit           close the terminal\n"
              "  force CMD      run a protected command anyway\n"
              "  Up/Down        command history     PgUp/PgDn or wheel: scroll\n"
              "  Left/Right/Home/End/Del  edit the line   Ctrl+C cancel   Ctrl+L cls\n");
}

static void run_command(const char *line){
    while(*line==' ')++line;
    if(!*line)return;
    char word[40]; uint32_t n=0;
    const char *p=line; while(*p&&*p!=' '&&n<sizeof(word)-1u)word[n++]=*p++;
    word[n]=0; while(*p&&*p!=' ')++p; while(*p==' ')++p;
    if(word_is(word,"cls")){clear_screen();return;}
    if(word_is(word,"exit")||word_is(word,"quit")){(void)psre_m036_stop(T.app);T.open=false;return;}
    bool forced=false;
    if(word_is(word,"force")){
        if(!*p){puts_term("Usage: force COMMAND\n");return;}
        line=p; forced=true; n=0; p=line;
        while(*p&&*p!=' '&&n<sizeof(word)-1u)word[n++]=*p++;
        word[n]=0;
    }
    if(!forced){
        const char *why=protected_reason(word);
        if(why){
            puts_term("[TERM] Blocked: ");puts_term(why);
            puts_term(".\n[TERM] Type 'force ");puts_term(word);puts_term("' to run it anyway.\n");
            ++T.status.commands_blocked; return;
        }
    }
    ++T.status.commands_run;
    if(word_is(word,"halt")){puts_term("System halted.\n");flush_screen();}
    else flush_screen();                 /* show the echoed command before long-running work */
    T.busy=true; serial_set_sink(sink);
    psre_m006_execute(line);
    serial_set_sink(0); T.busy=false;
    ensure_newline();
    if(word_is(word,"help"))extra_help();
}

static void echo_input(void){
    char pr[64]; make_prompt(pr);
    T.kind[T.count%LINES]=1;
    puts_term(pr); puts_term(T.input);
    ensure_newline();
}
static void history_add(void){
    if(!T.len)return;
    if(T.hist_count&&!(T.history[T.hist_count-1u][0]==0)){
        uint16_t i=0; while(T.history[T.hist_count-1u][i]&&T.history[T.hist_count-1u][i]==T.input[i])++i;
        if(!T.history[T.hist_count-1u][i]&&!T.input[i]){T.hist_cursor=T.hist_count;return;}
    }
    if(T.hist_count==HIST){
        for(uint8_t i=1;i<HIST;++i)for(uint16_t j=0;j<INPUT_MAX;++j)T.history[i-1u][j]=T.history[i][j];
        --T.hist_count;
    }
    for(uint16_t j=0;j<INPUT_MAX;++j)T.history[T.hist_count][j]=j<=T.len?T.input[j]:0;
    ++T.hist_count; T.hist_cursor=T.hist_count;
}
static void load_history(uint8_t idx){
    uint16_t n=0; while(idx<T.hist_count&&T.history[idx][n]&&n<INPUT_MAX-1u){T.input[n]=T.history[idx][n];++n;}
    T.input[n]=0; T.len=n; T.pos=n;
}
static void submit(void){
    echo_input(); history_add();
    char line[INPUT_MAX]; for(uint16_t i=0;i<=T.len;++i)line[i]=T.input[i];
    T.input[0]=0; T.len=0; T.pos=0; T.scroll=0; T.hist_cursor=T.hist_count;
    run_command(line);
}
void psre_m061_run_line(const char *text){
    if(!text)return;
    uint16_t n=0; while(text[n]&&n<INPUT_MAX-1u){T.input[n]=text[n];++n;}
    T.input[n]=0; T.len=n; T.pos=n; submit();
}

/* ---- lifecycle / input -------------------------------------------------- */
static void banner(void){
    puts_term("PSRE Terminal " PSRE_VERSION_STRING "\n"
              "Type 'help' for every command. Terminal: cls, exit, force CMD.\n"
              "History: Up/Down. Scroll: PgUp/PgDn or mouse wheel.\n\n");
}
bool psre_m061_open(psre_m036_app_id_t app){
    if(T.open&&T.app==app)return true;
    zero_state();
    T.app=app; T.open=true; T.cols=80; T.rows=20; T.hist_cursor=0;
    { geom_t g; if(compute_geometry(&g,0)){T.cols=(uint16_t)g.cols;T.rows=(uint16_t)g.rows;} }
    T.status.open=true; T.status.cols=T.cols; T.status.rows=T.rows;
    banner();
    return true;
}
bool psre_m061_on_event(psre_m036_app_id_t app,const psre_input_event_t *e,void *state){
    (void)app;(void)state;
    if(!T.open||T.busy||!e)return false;
    bool changed=false;
    if(e->type==PSRE_INPUT_MOUSE_BUTTON_DOWN||e->type==PSRE_INPUT_MOUSE_BUTTON_UP||e->type==PSRE_INPUT_MOUSE_MOVE){
        changed=scrollbar_mouse(e);
    } else if(e->type==PSRE_INPUT_MOUSE_WHEEL){
        if(e->wheel>0){T.scroll+=3u;changed=true;}
        else if(e->wheel<0){T.scroll=T.scroll>3u?T.scroll-3u:0u;changed=true;}
    } else if(e->type==PSRE_INPUT_KEY_DOWN||e->type==PSRE_INPUT_KEY_REPEAT){
        bool ctrl=(e->modifiers&MOD_CTRL)!=0, alt=(e->modifiers&MOD_ALT)!=0;
        if(ctrl&&e->code==0x06u){ /* Ctrl+C */
            char pr[64];make_prompt(pr);T.kind[T.count%LINES]=1;puts_term(pr);puts_term(T.input);puts_term("^C");ensure_newline();
            T.input[0]=0;T.len=0;T.pos=0;T.scroll=0;changed=true;
        } else if(ctrl&&e->code==0x0Fu){clear_screen();changed=true;}   /* Ctrl+L */
        else if(e->code==0x52u){ /* Up */
            if(T.hist_cursor>0){--T.hist_cursor;load_history(T.hist_cursor);changed=true;}
        } else if(e->code==0x51u){ /* Down */
            if(T.hist_cursor<T.hist_count){++T.hist_cursor;if(T.hist_cursor==T.hist_count){T.input[0]=0;T.len=0;T.pos=0;}else load_history(T.hist_cursor);changed=true;}
        } else if(e->code==0x50u){if(T.pos){--T.pos;changed=true;}}           /* Left */
        else if(e->code==0x4Fu){if(T.pos<T.len){++T.pos;changed=true;}}      /* Right */
        else if(e->code==0x4Au){T.pos=0;changed=true;}                        /* Home */
        else if(e->code==0x4Du){T.pos=T.len;changed=true;}                    /* End */
        else if(e->code==0x4Cu){                                             /* Delete */
            if(T.pos<T.len){for(uint16_t i=T.pos;i<T.len;++i)T.input[i]=T.input[i+1u];--T.len;changed=true;}
        } else if(e->code==0x4Bu){T.scroll+=T.rows>2u?(uint32_t)T.rows-1u:1u;changed=true;}   /* PgUp */
        else if(e->code==0x4Eu){uint32_t s=T.rows>2u?(uint32_t)T.rows-1u:1u;T.scroll=T.scroll>s?T.scroll-s:0u;changed=true;}
        else if(!ctrl&&!alt){
            char ch=e->character;
            if(ch=='\n'||ch=='\r'){submit();changed=true;}
            else if(ch=='\b'||ch==0x7f){
                if(T.pos){for(uint16_t i=T.pos;i<=T.len;++i)T.input[i-1u]=T.input[i];--T.pos;--T.len;T.scroll=0;changed=true;}
            } else if((unsigned char)ch>=0x20&&(unsigned char)ch<0x7f&&T.len+1u<INPUT_MAX){
                for(uint16_t i=T.len+1u;i>T.pos;--i)T.input[i]=T.input[i-1u];
                T.input[T.pos++]=ch;++T.len;T.scroll=0;changed=true;
            }
        }
    }
    if(changed){
        geom_t g;                                    /* clamps T.scroll to the scrollable range */
        if(!compute_geometry(&g,0)){uint32_t m=T.count>LINES-1u?LINES-1u:T.count;if(T.scroll>m)T.scroll=m;}
        (void)psre_m034_render();
    }
    return changed;
}

/* ---- geometry / rendering ----------------------------------------------- */
static uint32_t cell_h(void){return psre_m033_ui_large()?15u:10u;}
static bool draw_text(const char *s,int32_t x,int32_t y,uint32_t w,psre_m032_color_t fg){
    psre_m033_text_style_t st={.x=x,.y=y,.width=w,.height=12,.spacing=1,.font=PSRE_M033_FONT_8X8,
        .foreground=fg,.background={0,0,0},.opaque_background=false};
    return psre_m033_draw_string(s,&st);
}
static bool covered_by_other(const psre_m034_window_info_t *wi){
    for(uint8_t id=0;id<PSRE_M034_MAX_WINDOWS;++id){
        psre_m034_window_info_t o;
        if(id==wi->id||!psre_m034_get_window(id,&o)||!o.visible)continue;
        if(o.x<wi->x+wi->width&&o.x+o.width>wi->x&&o.y<wi->y+wi->height&&o.y+o.height>wi->y)return true;
    }
    return false;
}
static bool compute_geometry(geom_t *g,bool *focused){
    psre_m036_app_info_t ai;
    if(!psre_m036_get_app(T.app,&ai)||ai.lifecycle!=PSRE_M036_RUNNING)return false;
    psre_m034_window_info_t wi;
    if(!psre_m034_get_window(ai.window,&wi)||!wi.visible)return false;
    if(focused)*focused=wi.focused;
    g->bx=wi.x+WIN_BORDER; g->by=wi.y+WIN_TITLE+WIN_BORDER;
    g->bw=wi.width-2*WIN_BORDER; g->bh=wi.height-WIN_TITLE-2*WIN_BORDER;
    g->ch=cell_h();
    if(g->bw<CELL_W*10+2*PAD_X+SB_W||g->bh<(int32_t)g->ch*3+2*PAD_Y)return false;
    g->tx=g->bx+PAD_X; g->ty=g->by+PAD_Y;
    uint32_t cols=(uint32_t)(g->bw-2*PAD_X-SB_W-2)/CELL_W; if(cols>COLS)cols=COLS;
    g->cols=cols; g->rows=(uint32_t)(g->bh-2*PAD_Y)/g->ch;
    psre_m061_scrollbar_t *sb=&g->sb;
    sb->x=g->bx+g->bw-SB_W; sb->y=g->by; sb->w=SB_W; sb->h=g->bh;
    sb->up_y=g->by; sb->down_y=g->by+g->bh-SB_W;
    sb->track_y=g->by+SB_W; sb->track_h=g->bh-2*SB_W;
    sb->total=T.count+1u>LINES?LINES:(uint32_t)T.count+1u;
    sb->rows=g->rows;
    sb->max_scroll=sb->total>g->rows?sb->total-g->rows:0u;
    if(T.scroll>sb->max_scroll)T.scroll=sb->max_scroll;
    sb->scroll=T.scroll;
    int32_t th=sb->total>g->rows?(int32_t)((uint64_t)sb->track_h*g->rows/sb->total):sb->track_h;
    if(th<18)th=18;
    if(th>sb->track_h)th=sb->track_h;
    sb->thumb_h=th;
    int32_t travel=sb->track_h-th;
    sb->thumb_y=sb->track_y+(sb->max_scroll?(int32_t)((uint64_t)travel*(sb->max_scroll-sb->scroll)/sb->max_scroll):0);
    return true;
}
bool psre_m061_get_scrollbar(psre_m061_scrollbar_t *out){
    geom_t g; if(!T.open||!out||!compute_geometry(&g,0))return false;
    *out=g.sb; return true;
}
static void sb_set_from_thumb(const geom_t *g,int32_t thumb_top){
    const psre_m061_scrollbar_t *sb=&g->sb;
    int32_t travel=sb->track_h-sb->thumb_h;
    if(travel<=0||!sb->max_scroll){T.scroll=0;return;}
    if(thumb_top<sb->track_y)thumb_top=sb->track_y;
    if(thumb_top>sb->track_y+travel)thumb_top=sb->track_y+travel;
    uint32_t off=(uint32_t)(thumb_top-sb->track_y);
    T.scroll=sb->max_scroll-(uint32_t)(((uint64_t)off*sb->max_scroll+(uint32_t)travel/2u)/(uint32_t)travel);
}
/* Mouse handling for the scroll bar. Returns true if the event was consumed. */
static bool scrollbar_mouse(const psre_input_event_t *e){
    geom_t g; if(!compute_geometry(&g,0))return false;
    const psre_m034_status_t *wm=psre_m034_get_status();
    if(!wm)return false;
    int32_t cx=wm->cursor_x, cy=wm->cursor_y;
    const psre_m061_scrollbar_t *sb=&g.sb;
    if(e->type==PSRE_INPUT_MOUSE_BUTTON_UP){
        if(e->code==1u||!(e->buttons&1u)){bool was=T.sb_drag;T.sb_drag=false;return was;}
        return false;
    }
    if(e->type==PSRE_INPUT_MOUSE_MOVE){
        if(!T.sb_drag)return false;
        sb_set_from_thumb(&g,cy-T.sb_drag_off);return true;
    }
    if(e->type!=PSRE_INPUT_MOUSE_BUTTON_DOWN||!(e->code==1u||(e->buttons&1u)))return false;
    if(cx<sb->x||cx>=sb->x+sb->w||cy<sb->y||cy>=sb->y+sb->h)return false;
    uint32_t page=g.rows>2u?g.rows-1u:1u;
    if(cy<sb->up_y+sb->w){T.scroll+=1u;}                                   /* up arrow */
    else if(cy>=sb->down_y){T.scroll=T.scroll>0u?T.scroll-1u:0u;}           /* down arrow */
    else if(cy>=sb->thumb_y&&cy<sb->thumb_y+sb->thumb_h){T.sb_drag=true;T.sb_drag_off=cy-sb->thumb_y;} /* grab thumb */
    else if(cy<sb->thumb_y){T.scroll+=page;}                               /* track above: page up */
    else {T.scroll=T.scroll>page?T.scroll-page:0u;}                         /* track below: page down */
    return true;
}
static bool sb_button(int32_t x,int32_t y,bool up,bool active){
    psre_m032_color_t ink=active?rgb(205,215,230):rgb(95,105,120);
    bool ok=psre_m032_fill_rect(x,y,SB_W,SB_W,active?rgb(55,66,84):rgb(38,45,57));
    int32_t cx=x+SB_W/2;
    for(int32_t i=0;i<4;++i){                       /* small triangle, 1..7 px wide */
        int32_t half=up?i:3-i;
        ok=psre_m032_fill_rect(cx-half,y+5+i,(int32_t)(2*half+1),1,ink)&&ok;
    }
    return ok;
}
bool psre_m061_render(void){
    if(!T.open)return true;
    geom_t g; bool focused=false;
    psre_m036_app_info_t ai;
    if(!psre_m036_get_app(T.app,&ai)||ai.lifecycle!=PSRE_M036_RUNNING){T.open=false;T.status.open=false;return true;}
    psre_m034_window_info_t wi;
    if(!psre_m034_get_window(ai.window,&wi)||!wi.visible)return true;
    if(!wi.focused&&covered_by_other(&wi))return true;   /* let M034 show its plain body */
    (void)focused;
    if(!compute_geometry(&g,0))return true;
    bool ok=psre_m032_fill_rect(g.bx,g.by,g.bw,g.bh,rgb(12,16,22));
    T.cols=(uint16_t)g.cols; T.rows=(uint16_t)g.rows; T.status.cols=T.cols; T.status.rows=T.rows;
    T.status.scroll=g.sb.scroll; T.status.max_scroll=g.sb.max_scroll;
    uint32_t cols=g.cols, rows=g.rows, ch=g.ch;
    int64_t bottom=(int64_t)T.count-(int64_t)T.scroll;
    int64_t oldest=T.count>=LINES?(int64_t)T.count-(int64_t)(LINES-1u):0;
    char pr[64]; make_prompt(pr);
    uint32_t glyph_h=psre_m033_ui_large()?13u:8u;
    for(uint32_t r=0;r<rows;++r){
        int64_t idx=bottom-(int64_t)(rows-1u-r);
        if(idx<oldest||idx<0)continue;
        int32_t y=g.ty+(int32_t)(r*ch);
        if(idx==(int64_t)T.count){                       /* live prompt + input line */
            char tmp[COLS+1u]; uint32_t plen=0; while(pr[plen])++plen;
            uint32_t total=plen+T.len, caret=plen+T.pos;
            uint32_t start=caret>=cols?caret-cols+1u:0u;
            uint32_t k=0;
            for(uint32_t i=start;i<total&&k<cols;++i)tmp[k++]=i<plen?pr[i]:T.input[i-plen];
            tmp[k]=0;
            ok=draw_text(tmp,g.tx,y,cols*CELL_W,rgb(235,240,245))&&ok;
            if(start<plen){char pt[64];uint32_t m=0;for(uint32_t i=start;i<plen&&m<cols;++i)pt[m++]=pr[i];pt[m]=0;
                ok=draw_text(pt,g.tx,y,cols*CELL_W,rgb(110,200,230))&&ok;}
            ok=psre_m032_fill_rect(g.tx+(int32_t)((caret-start)*CELL_W),y+(int32_t)glyph_h,8,2,rgb(245,245,245))&&ok;
        } else {
            ok=draw_text(T.lines[idx%LINES],g.tx,y,cols*CELL_W,
                         T.kind[idx%LINES]?rgb(110,200,230):rgb(200,215,200))&&ok;
        }
    }
    /* ---- scroll bar ---- */
    const psre_m061_scrollbar_t *sb=&g.sb;
    ok=psre_m032_fill_rect(sb->x,sb->y,sb->w,sb->h,rgb(26,32,42))&&ok;
    ok=psre_m032_draw_line(sb->x,sb->y,sb->x,sb->y+sb->h-1,rgb(55,66,82))&&ok;
    ok=sb_button(sb->x,sb->up_y,true,T.scroll<sb->max_scroll)&&ok;
    ok=sb_button(sb->x,sb->down_y,false,T.scroll>0u)&&ok;
    psre_m032_color_t tc=T.sb_drag?rgb(140,160,190):rgb(95,112,135);
    ok=psre_m032_fill_rect(sb->x+2,sb->thumb_y,sb->w-4,sb->thumb_h,tc)&&ok;
    ok=psre_m032_fill_rect(sb->x+2,sb->thumb_y,sb->w-4,1,rgb(150,170,198))&&ok;
    if(T.scroll)ok=draw_text("[scrolled - type to return]",sb->x-27*CELL_W,g.by+2,26*CELL_W,rgb(230,200,90))&&ok;
    return ok;
}

bool psre_m061_copy_line(uint32_t back,char *out,uint32_t cap){
    if(!out||!cap||back>=T.count||back>=LINES-1u)return false;
    const char *s=T.lines[(T.count-1u-back)%LINES]; uint32_t n=0;
    while(s[n]&&n+1u<cap){out[n]=s[n];++n;}
    out[n]=0; return true;
}
const psre_m061_status_t *psre_m061_get_status(void){T.status.open=T.open;return &T.status;}
void psre_m061_report(void){
    const psre_m061_status_t *s=psre_m061_get_status();
    serial_write("[M061] GUI terminal open=");serial_write(s->open?"yes":"no");
    char b[12];uint32_t v=s->commands_run;uint8_t n=0;
    serial_write(" commands=");do{b[n++]=(char)('0'+v%10u);v/=10u;}while(v&&n<sizeof(b));while(n)serial_write_char(b[--n]);
    v=s->commands_blocked;n=0;serial_write(" blocked=");do{b[n++]=(char)('0'+v%10u);v/=10u;}while(v&&n<sizeof(b));while(n)serial_write_char(b[--n]);
    serial_write("\n");
}
