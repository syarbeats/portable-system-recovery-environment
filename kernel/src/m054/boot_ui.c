#include <psre/m054/boot_ui.h>
#include <psre/m031/framebuffer.h>
#include <psre/m032/graphics.h>
#include <psre/m033/font.h>
#include <psre/serial.h>

#define LOG_ROWS 22u
#define LOG_COLS 96u
static bool ready;
static char rows[LOG_ROWS][LOG_COLS];
static uint32_t total, shown;
static psre_m032_color_t rgb(uint8_t r,uint8_t g,uint8_t b){psre_m032_color_t c={r,g,b};return c;}
static bool draw_text(const char *s,int32_t x,int32_t y,uint32_t w,uint32_t h,psre_m032_color_t fg){
    psre_m033_text_style_t st={.x=x,.y=y,.width=w,.height=h,.spacing=1,.font=PSRE_M033_FONT_8X8,.foreground=fg,.background={8,13,22},.opaque_background=false};
    return psre_m033_draw_string(s,&st);
}
static void redraw(void){
    if(!ready)return;
    const psre_m031_framebuffer_t *fb=psre_m031_get();
    (void)psre_m032_clear(rgb(8,13,22));
    (void)psre_m032_fill_rect(0,0,(int32_t)fb->width,34,rgb(18,43,68));
    (void)draw_text("PSRE - SYSTEM STARTUP",14,12,(uint32_t)fb->width-28u,12,rgb(235,244,255));
    (void)draw_text("Kernel initialization log  |  UEFI / Limine  |  x86_64",14,44,(uint32_t)fb->width-28u,12,rgb(130,190,225));
    uint32_t max_rows=(uint32_t)((fb->height>90u?(fb->height-78u)/18u:1u));
    if(max_rows>LOG_ROWS)max_rows=LOG_ROWS;
    uint32_t first=shown>max_rows?shown-max_rows:0;
    for(uint32_t i=first;i<shown;++i){
        uint32_t row=i-first;
        (void)draw_text(rows[i],14,68+(int32_t)(row*18u),(uint32_t)fb->width-28u,12,
                        (i+1u==shown)?rgb(220,238,250):rgb(170,190,208));
    }
    (void)psre_m032_fill_rect(0,(int32_t)fb->height-24,(int32_t)fb->width,24,rgb(18,29,44));
    (void)draw_text("PSRE boot diagnostics - status is also mirrored to serial COM1",12,(int32_t)fb->height-16,(uint32_t)fb->width-24u,10,rgb(115,160,195));
}
void psre_m054_boot_ui_begin(void){
    ready=false; total=shown=0;
    const psre_m031_framebuffer_t *fb=psre_m031_get();
    if(!fb||!fb->available||!psre_m032_initialize()||!psre_m033_initialize())return;
    ready=true;
    (void)psre_m032_clear(rgb(8,13,22));
    psre_m054_boot_ui_step("Framebuffer graphics console initialized","OK");
}
void psre_m054_boot_ui_step(const char *name,const char *result){
    serial_write("[BOOT] ");serial_write(name?name:"step");serial_write(" ... ");serial_write(result?result:"INFO");serial_write("\n");
    if(!ready||!name)return;
    uint32_t slot=total%LOG_ROWS; uint32_t n=0;
    const char *prefix="[  OK  ] ";
    if(result && (result[0]=='W'||result[0]=='F'))prefix=result[0]=='W'?"[ WARN ] ":"[ FAIL ] ";
    while(prefix[n]&&n<LOG_COLS-1u){rows[slot][n]=prefix[n];++n;}
    for(uint32_t i=0;name[i]&&n<LOG_COLS-1u;++i)rows[slot][n++]=name[i];
    rows[slot][n]=0;
    ++total; shown=total<LOG_ROWS?total:LOG_ROWS;
    /* Keep chronological order after wrap by rotating the retained rows. */
    if(total>LOG_ROWS){
        char tmp[LOG_ROWS][LOG_COLS];
        for(uint32_t i=0;i<LOG_ROWS;++i){uint32_t idx=(total+i)%LOG_ROWS;for(uint32_t j=0;j<LOG_COLS;++j)tmp[i][j]=rows[idx][j];}
        for(uint32_t i=0;i<LOG_ROWS;++i)for(uint32_t j=0;j<LOG_COLS;++j)rows[i][j]=tmp[i][j];
    }
    redraw();
}
void psre_m054_boot_ui_desktop_notice(const char *message){
    if(!ready||!message)return;
    (void)psre_m032_clear(rgb(18,39,64));
    (void)draw_text("PSRE DESKTOP",24,24,400,20,rgb(245,250,255));
    (void)draw_text(message,24,56,700,24,rgb(205,225,245));
    (void)draw_text("System boot completed. USB input may initialize independently.",24,82,900,20,rgb(155,190,220));
}
bool psre_m054_boot_ui_available(void){return ready;}
