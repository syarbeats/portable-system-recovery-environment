#include <psre/m042/file_explorer.h>
#include <psre/m043/process.h>
#include <psre/m006/fs.h>
#include <psre/serial.h>
#include <stddef.h>

enum { ACT_UP=0, ACT_REFRESH, ACT_PREV, ACT_NEXT, ACT_VIEW, ACT_OPEN,
       ACT_STAGE, ACT_QUEUE_ADD, ACT_QUEUE_EXPORT, ACT_QUEUE_CLEAR };
static psre_m042_status_t state;
static psre_m038_widget_id_t path_widget, status_widget, select_widget;
static psre_m038_widget_id_t buttons[10];
static psre_m040_control_id_t table_id, progress_id;
static psre_m007_dir_entry_t directory[PSRE_M007_MAX_DIRECTORY_ENTRIES];
static char current_path[PSRE_FS_MAX_PATH];
static const char *button_names[10]={
    "Up","Refresh","Prev","Next","Queue","Open","Stage",
    "Add Q","Export","ClearQ"
};
static void cp(char*d,const char*s,uint32_t cap){uint32_t i=0;if(!d||!cap)return;while(s&&s[i]&&i+1u<cap){d[i]=s[i];++i;}d[i]=0;}
static void dec(uint32_t v,char out[12]){char t[11];uint8_t n=0;do{t[n++]=(char)('0'+v%10u);v/=10u;}while(v&&n<10);uint8_t j=0;while(n)out[j++]=t[--n];out[j]=0;}
static void set_status(const char*s){if(state.initialized)(void)psre_m038_set_text(state.container,status_widget,s);}
static bool read_path(char out[PSRE_FS_MAX_PATH]){
    if(!psre_m038_get_text(state.container,path_widget,out,PSRE_FS_MAX_PATH))return false;
    if(!out[0])cp(out,"/",PSRE_FS_MAX_PATH);
    return true;
}
static void refresh_progress(void){
    (void)psre_m009_queue_get_info(&state.queue_count,&state.queue_bytes);
    (void)psre_m040_set_range(progress_id,0,131072);
    (void)psre_m040_set_value(progress_id,(int32_t)state.queue_bytes);
}
static void clear_table(void){(void)psre_m040_clear_table(table_id);}
static void set_row(uint8_t row,const char*a,const char*b,const char*c){
    (void)psre_m040_set_table_cell(table_id,row,0,a);
    (void)psre_m040_set_table_cell(table_id,row,1,b);
    (void)psre_m040_set_table_cell(table_id,row,2,c);
}
static void update_selection(void){
    state.selected_valid=false;state.selected_checked=false;
    (void)psre_m038_set_checked(state.container,select_widget,false);
}
bool psre_m042_refresh(void){
    if(!state.initialized)return false;
    char path[PSRE_FS_MAX_PATH];
    if(!read_path(path)){++state.errors;set_status("Invalid path textbox.");return false;}
    cp(current_path,path,sizeof(current_path));
    update_selection();clear_table();
    state.mounted=psre_fat32_is_mounted();
    if(state.view==PSRE_M042_FILES){
        uint32_t count=0,total=0;
        if(!psre_m007_read_dir(current_path,directory,PSRE_M007_MAX_DIRECTORY_ENTRIES,&count,&total)){
            state.directory_count=state.directory_total=0;++state.errors;
            set_status("Directory read failed. Check FAT32 mount and path.");return false;
        }
        state.directory_count=count;state.directory_total=total;
        uint32_t start=(uint32_t)state.page*PSRE_M042_PAGE_SIZE;
        for(uint8_t row=0;row<PSRE_M042_PAGE_SIZE;++row){
            uint32_t index=start+row;
            if(index>=count)break;
            psre_m007_dir_entry_t *e=&directory[index];
            char size[12];dec(e->size,size);
            set_row(row,e->name,(e->attributes&0x10u)?"DIR":"FILE",size);
        }
        char msg[96],n[12],t[12];dec(total,n);dec(count,t);
        cp(msg,"Directory loaded: ",sizeof(msg));
        uint32_t pos=19;for(uint32_t i=0;t[i]&&pos+1<sizeof(msg);++i)msg[pos++]=t[i];
        const char suffix[]=" entries cached / ";for(uint32_t i=0;suffix[i]&&pos+1<sizeof(msg);++i)msg[pos++]=suffix[i];
        for(uint32_t i=0;n[i]&&pos+1<sizeof(msg);++i)msg[pos++]=n[i];
        const char end[]=" total";for(uint32_t i=0;end[i]&&pos+1<sizeof(msg);++i)msg[pos++]=end[i];
        msg[pos]=0;set_status(msg);
    }else{
        uint32_t count=0,used=0;
        if(!psre_m009_queue_get_info(&count,&used)){++state.errors;return false;}
        state.queue_count=count;state.queue_bytes=used;
        uint32_t start=(uint32_t)state.page*PSRE_M042_PAGE_SIZE;
        for(uint8_t row=0;row<PSRE_M042_PAGE_SIZE;++row){
            psre_m009_queue_entry_t item;
            if(start+row>=count)break;
            if(!psre_m009_queue_get_entry(start+row,&item))continue;
            char size[12];dec(item.size,size);
            set_row(row,item.path,"QUEUED",size);
        }
        char msg[96],a[12],b[12];dec(count,a);dec(used,b);
        cp(msg,"Recovery queue: ",sizeof(msg));uint32_t pos=16;
        for(uint32_t i=0;a[i]&&pos+1<sizeof(msg);++i)msg[pos++]=a[i];
        const char mid[]=" files / ";for(uint32_t i=0;mid[i]&&pos+1<sizeof(msg);++i)msg[pos++]=mid[i];
        for(uint32_t i=0;b[i]&&pos+1<sizeof(msg);++i)msg[pos++]=b[i];
        const char end[]=" bytes";for(uint32_t i=0;end[i]&&pos+1<sizeof(msg);++i)msg[pos++]=end[i];
        msg[pos]=0;set_status(msg);
    }
    refresh_progress();++state.refreshes;
    (void)psre_m040_render_container(state.container);
    return true;
}
bool psre_m042_set_path(const char*path){
    if(!state.initialized||!path||!*path)return false;
    uint32_t n=0;while(path[n]&&n<PSRE_M038_TEXT_CAPACITY)++n;
    if(n>=PSRE_M038_TEXT_CAPACITY)return false;
    if(!psre_m038_set_text(state.container,path_widget,path))return false;
    state.page=0;return psre_m042_refresh();
}
bool psre_m042_set_view(psre_m042_view_t view){
    if(!state.initialized||view>PSRE_M042_QUEUE)return false;
    state.view=view;state.page=0;
    (void)psre_m038_set_enabled(state.container,select_widget,view==PSRE_M042_FILES);
    return psre_m042_refresh();
}
static bool selected_file(psre_m007_dir_entry_t **entry){
    if(!state.selected_valid||state.view!=PSRE_M042_FILES||state.selected_index>=state.directory_count)return false;
    psre_m007_dir_entry_t *e=&directory[state.selected_index];
    if(e->attributes&0x10u)return false;
    if(entry) *entry=e;
    return true;
}
static bool selected_path(char out[PSRE_FS_MAX_PATH]){
    psre_m007_dir_entry_t *e;if(!selected_file(&e))return false;
    uint32_t n=0;while(current_path[n]&&n<PSRE_FS_MAX_PATH-1u){out[n]=current_path[n];++n;}
    if(n>1&&out[n-1]!='/')out[n++]='/';
    uint32_t j=0;while(e->name[j]&&n+1u<PSRE_FS_MAX_PATH)out[n++]=e->name[j++];
    out[n]=0;return true;
}
bool psre_m042_open_selected(void){
    if(!state.initialized||state.view!=PSRE_M042_FILES||!state.selected_valid)return false;
    psre_m007_dir_entry_t *e=&directory[state.selected_index];
    if(!(e->attributes&0x10u)){set_status("Selected entry is a file; use Stage RAM or Queue Add.");return false;}
    char next[PSRE_FS_MAX_PATH];uint32_t n=0;
    while(current_path[n]&&n<PSRE_FS_MAX_PATH-1u){next[n]=current_path[n];++n;}
    if(n>1&&next[n-1]!='/')next[n++]='/';
    uint32_t j=0;while(e->name[j]&&n+1u<PSRE_FS_MAX_PATH)next[n++]=e->name[j++];
    next[n]=0;
    return psre_m042_set_path(next);
}
bool psre_m042_stage_selected(void){
    char path[PSRE_FS_MAX_PATH];
    if(!state.initialized||!state.selected_checked||!selected_path(path)){
        set_status("Select a file row and tick Select highlighted file.");return false;
    }
    bool ok=psre_m043_start_stage(path);
    if(ok){++state.stage_operations;set_status("M043 staging job started; use m035-poll to advance.");}
    else{++state.errors;set_status("Staging job could not start; see serial diagnostics.");}
    return ok;
}
bool psre_m042_queue_selected(void){
    char path[PSRE_FS_MAX_PATH];
    if(!state.initialized||!state.selected_checked||!selected_path(path)){
        set_status("Select a file row and tick Select highlighted file.");return false;
    }
    bool ok=psre_m009_queue_add(path);
    if(ok){++state.queue_operations;}
    else{++state.errors;}
    refresh_progress();(void)psre_m042_refresh();
    set_status(ok?"File added to recovery queue.":"Queue add failed; see serial diagnostics.");
    return ok;
}
static void action(psre_m038_container_id_t c,psre_m038_widget_id_t w,void*ctx){
    (void)c;(void)w;uintptr_t a=(uintptr_t)ctx;
    switch(a){
    case ACT_UP:{
        char path[PSRE_FS_MAX_PATH];
        if(!read_path(path))cp(path,current_path,sizeof(path));
        uint32_t n=0;while(path[n])++n;
        while(n>1&&path[n-1]!='/')--n;
        if(n>1) --n;
        path[n]=0;
        (void)psre_m042_set_path(path);break;
    }
    case ACT_REFRESH:(void)psre_m042_refresh();break;
    case ACT_PREV:if(state.page){--state.page;(void)psre_m042_refresh();}break;
    case ACT_NEXT:{
        uint32_t total=state.view==PSRE_M042_FILES?state.directory_total:state.queue_count;
        if(state.view==PSRE_M042_FILES && total>state.directory_count)total=state.directory_count;
        uint32_t pages=(total+PSRE_M042_PAGE_SIZE-1u)/PSRE_M042_PAGE_SIZE;
        if((uint32_t)(state.page+1u)<pages){++state.page;(void)psre_m042_refresh();}break;
    }
    case ACT_VIEW:(void)psre_m042_set_view(state.view==PSRE_M042_FILES?PSRE_M042_QUEUE:PSRE_M042_FILES);break;
    case ACT_OPEN:(void)psre_m042_open_selected();break;
    case ACT_STAGE:(void)psre_m042_stage_selected();break;
    case ACT_QUEUE_ADD:(void)psre_m042_queue_selected();break;
    case ACT_QUEUE_EXPORT:
        if(psre_m009_queue_export())set_status("Queue export sent over serial HEX.");
        else set_status("Queue export failed or queue is empty.");
        break;
    case ACT_QUEUE_CLEAR:
        psre_m009_queue_clear();(void)psre_m042_refresh();set_status("Recovery queue cleared.");break;
    default:break;
    }
}
bool psre_m042_initialize(psre_m038_container_id_t container){
    psre_m038_container_info_t ci;if(!psre_m038_get_container(container,&ci)||!ci.used)return false;
    state=(psre_m042_status_t){.container=container,.view=PSRE_M042_FILES};
    table_id=psre_m040_create(container,PSRE_M040_TABLE,"NAME                 TYPE       SIZE",10,100,490,138);
    progress_id=psre_m040_create(container,PSRE_M040_PROGRESS,"Queue RAM (128 KiB)",370,252,130,20);
    if(table_id==PSRE_M040_INVALID_CONTROL||progress_id==PSRE_M040_INVALID_CONTROL)return false;
    state.table=table_id;state.progress=progress_id;
    (void)psre_m040_set_range(progress_id,0,131072);
    (void)psre_m040_set_value(progress_id,0);
    (void)psre_m040_set_value(table_id,-1);
    (void)psre_m040_set_table_cell(table_id,0,0,"NAME");
    path_widget=psre_m038_add_textbox(container,"/",10,10,230,24,PSRE_M038_TEXT_CAPACITY-1u);
    if(path_widget==PSRE_M038_INVALID_WIDGET)return false;
    select_widget=psre_m038_add_checkbox(container,"Select highlighted file",10,74,180,false);
    if(select_widget==PSRE_M038_INVALID_WIDGET)return false;
    status_widget=psre_m038_add_label(container,"Initializing FAT32 Explorer...",10,240,340);
    if(status_widget==PSRE_M038_INVALID_WIDGET)return false;
    for(uint8_t i=0;i<10;++i){
        static const int32_t top_x[4]={245,290,361,414};
        static const int32_t top_w[4]={42,66,48,48};
        int32_t bx=(i<4)?top_x[i]:10+(int32_t)(i-4)*82;
        int32_t by=(i<4)?10:42;
        int32_t bw=(i<4)?top_w[i]:78;
        buttons[i]=psre_m038_add_button(container,button_names[i],bx,by,bw,24,action,(void*)(uintptr_t)i);
        if(buttons[i]==PSRE_M038_INVALID_WIDGET)return false;
    }
    state.initialized=true;
    state.mounted=psre_fat32_is_mounted();
    cp(current_path,"/",sizeof(current_path));
    (void)psre_m038_set_text(container,path_widget,current_path);
    serial_write("[M042] GUI File Explorer initialized.\n");
    if(!state.mounted){set_status("FAT32 not mounted. Use M041 Mount Volume or mount from CLI.");return true;}
    return psre_m042_refresh();
}
bool psre_m042_handle_event(const psre_input_event_t*event){
    if(!state.initialized||!event)return false;
    if(event->type==PSRE_INPUT_MOUSE_BUTTON_DOWN){
        uint8_t row;
        if(psre_m040_get_table_selection(table_id,&row)){
            state.selected_index=(uint8_t)((uint32_t)state.page*PSRE_M042_PAGE_SIZE+row);
            state.selected_valid=(state.view==PSRE_M042_FILES)?
                state.selected_index<state.directory_count:state.selected_index<state.queue_count;
            state.selected_checked=false;
            (void)psre_m038_set_checked(state.container,select_widget,false);
        }
    }
    bool checked=false;
    if(psre_m038_is_checked(state.container,select_widget,&checked)){
        state.selected_checked=checked;
    }
    return false;
}
const psre_m042_status_t*psre_m042_get_status(void){return &state;}
static void num(uint32_t v){char b[12];dec(v,b);serial_write(b);}
void psre_m042_report(void){
    serial_write("[M042] initialized=");serial_write(state.initialized?"yes":"no");
    serial_write(" view=");serial_write(state.view==PSRE_M042_FILES?"FILES":"QUEUE");
    serial_write(" entries=");num(state.directory_total);
    serial_write(" queue_files=");num(state.queue_count);
    serial_write(" queue_bytes=");num(state.queue_bytes);
    serial_write("\n");
}
