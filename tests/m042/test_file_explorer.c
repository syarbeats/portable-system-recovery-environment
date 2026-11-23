#include <psre/m042/file_explorer.h>
#include <psre/m043/process.h>
#include <psre/m006/fs.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static char widgets[16][128];
static bool checks[16];
static uint8_t selected_row=0;
static bool selection_valid=false;
static char cells[12][3][64];
static uint8_t table_rows;
static int staged,queued,exported,cleared;
static uint32_t qcount,qbytes;
static psre_m009_queue_entry_t qitems[16];
static psre_m007_dir_entry_t sample[10];

bool psre_m038_get_container(psre_m038_container_id_t id,psre_m038_container_info_t*out){(void)id;*out=(psre_m038_container_info_t){.used=true};return true;}
psre_m038_widget_id_t psre_m038_add_textbox(psre_m038_container_id_t c,const char*t,int32_t x,int32_t y,int32_t w,int32_t h,uint16_t m){(void)c;(void)x;(void)y;(void)w;(void)h;(void)m;strncpy(widgets[0],t,127);return 0;}
psre_m038_widget_id_t psre_m038_add_label(psre_m038_container_id_t c,const char*t,int32_t x,int32_t y,uint32_t w){(void)c;(void)t;(void)x;(void)y;(void)w;static uint8_t id=1;return id++;}
psre_m038_widget_id_t psre_m038_add_button(psre_m038_container_id_t c,const char*t,int32_t x,int32_t y,int32_t w,int32_t h,psre_m038_action_t a,void*ctx){(void)c;(void)t;(void)x;(void)y;(void)w;(void)h;(void)a;(void)ctx;static uint8_t id=3;return id++;}
psre_m038_widget_id_t psre_m038_add_checkbox(psre_m038_container_id_t c,const char*t,int32_t x,int32_t y,uint32_t w,bool checked){(void)c;(void)t;(void)x;(void)y;(void)w;checks[2]=checked;return 2;}
bool psre_m038_set_text(psre_m038_container_id_t c,psre_m038_widget_id_t id,const char*t){(void)c;if(id>=16)return false;strncpy(widgets[id],t,127);return true;}
bool psre_m038_get_text(psre_m038_container_id_t c,psre_m038_widget_id_t id,char*out,uint32_t cap){(void)c;if(id>=16||!cap)return false;strncpy(out,widgets[id],cap-1);out[cap-1]=0;return true;}
bool psre_m038_set_checked(psre_m038_container_id_t c,psre_m038_widget_id_t id,bool v){(void)c;checks[id]=v;return true;}
bool psre_m038_is_checked(psre_m038_container_id_t c,psre_m038_widget_id_t id,bool*out){(void)c;*out=checks[id];return true;}
bool psre_m038_set_enabled(psre_m038_container_id_t c,psre_m038_widget_id_t id,bool enabled){(void)c;(void)id;(void)enabled;return true;}
psre_m040_control_id_t psre_m040_create(psre_m038_container_id_t c,psre_m040_type_t t,const char*title,int32_t x,int32_t y,int32_t w,int32_t h){(void)c;(void)t;(void)title;(void)x;(void)y;(void)w;(void)h;static uint8_t id=0;return id++;}
bool psre_m040_set_range(psre_m040_control_id_t id,int32_t a,int32_t b){(void)id;(void)a;(void)b;return true;}
bool psre_m040_set_value(psre_m040_control_id_t id,int32_t v){(void)id;(void)v;return true;}
bool psre_m040_clear_table(psre_m040_control_id_t id){(void)id;table_rows=0;memset(cells,0,sizeof(cells));return true;}
bool psre_m040_set_table_cell(psre_m040_control_id_t id,uint8_t r,uint8_t c,const char*s){(void)id;strncpy(cells[r][c],s,63);if(table_rows<=r)table_rows=r+1;return true;}
bool psre_m040_get_table_selection(psre_m040_control_id_t id,uint8_t*r){(void)id;if(!selection_valid)return false;selection_valid=false;*r=selected_row;return true;}
bool psre_m040_render_container(psre_m038_container_id_t id){(void)id;return true;}
bool psre_fat32_is_mounted(void){return true;}
bool psre_m007_read_dir(const char*path,psre_m007_dir_entry_t*e,uint32_t cap,uint32_t*count,uint32_t*total){
    if(strcmp(path,"/")&&strcmp(path,"/DOCS"))return false;
    uint32_t n=strcmp(path,"/DOCS")?10u:2u;
    *count=n<cap?n:cap;*total=n;
    for(uint32_t i=0;i<*count;++i)e[i]=sample[i];
    return true;
}
bool psre_m043_start_stage(const char*p){assert(strcmp(p,"/TEST.TXT")==0);++staged;return true;}
bool psre_m009_queue_add(const char*p){assert(strcmp(p,"/TEST.TXT")==0);++queued;qitems[0]=(psre_m009_queue_entry_t){.size=123,.crc32=0xAABBCCDD};strcpy(qitems[0].path,p);qcount=1;qbytes=123;return true;}
bool psre_m009_queue_get_info(uint32_t*c,uint32_t*b){*c=qcount;*b=qbytes;return true;}
bool psre_m009_queue_get_entry(uint32_t i,psre_m009_queue_entry_t*out){if(i>=qcount)return false;*out=qitems[i];return true;}
bool psre_m009_queue_export(void){++exported;return qcount!=0;}
void psre_m009_queue_clear(void){++cleared;qcount=qbytes=0;}
void serial_write(const char*s){(void)s;}
void serial_write_char(char c){(void)c;}

int main(void){
    strcpy(sample[0].name,"DOCS");sample[0].attributes=0x10;
    strcpy(sample[1].name,"TEST.TXT");sample[1].size=123;sample[1].attributes=0x20;
    for(unsigned i=2;i<10;++i){snprintf(sample[i].name,sizeof(sample[i].name),"F%02u.BIN",i);sample[i].size=i*10;sample[i].attributes=0x20;}
    assert(psre_m042_initialize(0));
    const psre_m042_status_t*st=psre_m042_get_status();
    assert(st->initialized&&st->directory_total==10&&table_rows==6);
    assert(strcmp(cells[0][0],"DOCS")==0&&strcmp(cells[1][0],"TEST.TXT")==0);
    selected_row=1;selection_valid=true;
    psre_input_event_t mouse={.type=PSRE_INPUT_MOUSE_BUTTON_DOWN};
    (void)psre_m042_handle_event(&mouse);
    assert(st->selected_valid&&!st->selected_checked);
    checks[2]=true;psre_input_event_t key={.type=PSRE_INPUT_KEY_DOWN};
    (void)psre_m042_handle_event(&key);
    assert(psre_m042_stage_selected()&&staged==1);
    assert(psre_m042_queue_selected()&&queued==1);
    assert(st->queue_count==1&&st->queue_bytes==123);
    assert(psre_m042_set_view(PSRE_M042_QUEUE));
    assert(strcmp(cells[0][0],"/TEST.TXT")==0);
    assert(psre_m042_set_view(PSRE_M042_FILES));
    selected_row=0;selection_valid=true;(void)psre_m042_handle_event(&mouse);
    assert(psre_m042_open_selected());
    assert(strcmp(widgets[0],"/DOCS")==0);
    assert(psre_m042_set_path("/"));
    assert(st->directory_total==10);
    puts("M042 File Explorer tests: PASS");
    return 0;
}
