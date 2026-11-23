#include <psre/m041/recovery_manager.h>
#include <psre/m043/process.h>
#include <psre/m006/fs.h>
#include <psre/serial.h>
#include <stddef.h>

static psre_m041_status_t state;
static psre_m038_widget_id_t buttons[13];
static const char *button_text[13]={
    "Mount Volume","List Directory","File Metadata","Calculate CRC32",
    "Recover to RAM","Add to Queue","Queue List","Export Queue",
    "Clear Queue","Workspace Manifest","Export Workspace",
    "Pause / Resume Job","Retry Failed Job"
};
static const psre_m041_action_t button_actions[13]={
    PSRE_M041_ACTION_MOUNT,PSRE_M041_ACTION_LIST,PSRE_M041_ACTION_STAT,
    PSRE_M041_ACTION_CRC32,PSRE_M041_ACTION_RECOVER,PSRE_M041_ACTION_QUEUE_ADD,
    PSRE_M041_ACTION_QUEUE_LIST,PSRE_M041_ACTION_QUEUE_EXPORT,
    PSRE_M041_ACTION_QUEUE_CLEAR,PSRE_M041_ACTION_MANIFEST,PSRE_M041_ACTION_EXPORT,
    PSRE_M041_ACTION_NONE,PSRE_M041_ACTION_NONE
};
static void copy_text(char *dst,const char *src,uint32_t cap){
    uint32_t i=0;if(!dst||!cap)return;
    while(src&&src[i]&&i+1<cap){dst[i]=src[i];++i;}dst[i]=0;
}
static void set_status(const char *s){
    if(state.initialized && state.status_widget!=PSRE_M038_INVALID_WIDGET)
        (void)psre_m038_set_text(state.container,state.status_widget,s);
}
static void action_callback(psre_m038_container_id_t c,psre_m038_widget_id_t w,void *ctx){
    (void)c;(void)w;
    uintptr_t n=(uintptr_t)ctx;
    if(n<11u)(void)psre_m041_execute(button_actions[n]);
    else if(n==11u){const psre_m043_status_t *s=psre_m043_get_status();if(s->state==PSRE_M043_RUNNING)(void)psre_m043_pause();else if(s->state==PSRE_M043_PAUSED)(void)psre_m043_resume();}
    else if(n==12u)(void)psre_m043_retry();
}
void psre_m041_reset(void){
    state=(psre_m041_status_t){.container=PSRE_M038_INVALID_CONTAINER,
      .path_widget=PSRE_M038_INVALID_WIDGET,.status_widget=PSRE_M038_INVALID_WIDGET};
    for(uint32_t i=0;i<13;++i)buttons[i]=PSRE_M038_INVALID_WIDGET;
}
bool psre_m041_initialize(psre_m038_container_id_t container){
    psre_m038_container_info_t ci;
    if(!psre_m038_get_container(container,&ci)||!ci.used)return false;
    psre_m041_reset();
    state.container=container;
    state.path_widget=psre_m038_add_textbox(container,"/",10,10,260,26,PSRE_M041_PATH_CAPACITY-1u);
    if(state.path_widget==PSRE_M038_INVALID_WIDGET)return false;
    (void)psre_m038_add_label(container,"FAT32 / 8.3 paths | Source media: READ ONLY",10,42,330);
    state.status_widget=psre_m038_add_label(container,"Ready. Mount a FAT32 volume to begin.",10,72,340);
    if(state.status_widget==PSRE_M038_INVALID_WIDGET)return false;
    for(uint32_t i=0;i<13;++i){
        int32_t col=(int32_t)(i%2u),row=(int32_t)(i/2u);
        buttons[i]=psre_m038_add_button(container,button_text[i],10+col*166,104+row*27,156,24,action_callback,(void *)(uintptr_t)i);
        if(buttons[i]==PSRE_M038_INVALID_WIDGET)return false;
    }
    if(!psre_m043_initialize(container))return false;
    state.initialized=true;
    state.mounted=psre_fat32_is_mounted();
    copy_text(state.current_path,"/",sizeof(state.current_path));
    set_status(state.mounted?"FAT32 volume already mounted.":"Ready. Select Mount Volume.");
    serial_write("[M041] Recovery Manager GUI initialized.\n");
    return true;
}
bool psre_m041_set_path(const char *path){
    if(!state.initialized||!path||!*path)return false;
    uint32_t n=0;while(path[n]&&n<PSRE_M041_PATH_CAPACITY)++n;
    if(n>=PSRE_M041_PATH_CAPACITY)return false;
    copy_text(state.current_path,path,sizeof(state.current_path));
    return psre_m038_set_text(state.container,state.path_widget,path);
}
bool psre_m041_execute(psre_m041_action_t a){
    if(!state.initialized)return false;
    char path[PSRE_M041_PATH_CAPACITY]="/";
    bool needs_path=(a==PSRE_M041_ACTION_LIST||a==PSRE_M041_ACTION_STAT||
        a==PSRE_M041_ACTION_CRC32||a==PSRE_M041_ACTION_RECOVER||
        a==PSRE_M041_ACTION_QUEUE_ADD);
    if(needs_path && (!psre_m038_get_text(state.container,state.path_widget,path,sizeof(path))||!path[0])){
        set_status("Enter a FAT 8.3 path first.");++state.failures;return false;
    }
    ++state.actions;state.last_action=a;
    bool ok=false;
    switch(a){
    case PSRE_M041_ACTION_MOUNT:
        ok=psre_fat32_mount_first();state.mounted=ok;
        set_status(ok?"FAT32 mounted read-only.":"Mount failed; inspect storage diagnostics.");
        break;
    case PSRE_M041_ACTION_LIST:
        if(!state.mounted)state.mounted=psre_fat32_is_mounted();
        ok=state.mounted&&psre_m007_list_dir(path);
        set_status(ok?"Directory listing sent to serial console.":"Directory listing failed; check path/serial log.");
        break;
    case PSRE_M041_ACTION_STAT:
        ok=state.mounted&&psre_m007_stat(path);
        set_status(ok?"File metadata sent to serial console.":"Metadata lookup failed.");
        break;
    case PSRE_M041_ACTION_CRC32:
        ok=state.mounted&&psre_m007_crc32(path);
        set_status(ok?"CRC32 result sent to serial console.":"CRC32 calculation failed.");
        break;
    case PSRE_M041_ACTION_RECOVER:
        ok=state.mounted&&psre_m043_start_stage(path);
        set_status(ok?"M043 staging job started; use m035-poll to advance.":"Recovery job could not start.");
        break;
    case PSRE_M041_ACTION_QUEUE_ADD:
        ok=state.mounted&&psre_m009_queue_add(path);
        set_status(ok?"File added to recovery queue.":"Queue add failed or queue capacity reached.");
        break;
    case PSRE_M041_ACTION_QUEUE_LIST:
        psre_m009_queue_list();ok=true;set_status("Queue listing sent to serial console.");break;
    case PSRE_M041_ACTION_QUEUE_EXPORT:
        ok=psre_m043_start_queue_export();set_status(ok?"M043 export job started; use m035-poll to advance.":"Export job could not start.");break;
    case PSRE_M041_ACTION_QUEUE_CLEAR:
        psre_m009_queue_clear();ok=true;set_status("Recovery queue cleared.");break;
    case PSRE_M041_ACTION_MANIFEST:
        psre_m009_queue_manifest();psre_m008_manifest();ok=true;set_status("Recovery manifests sent to serial console.");break;
    case PSRE_M041_ACTION_WORKSPACE:
        psre_m008_workspace_info();ok=true;set_status("RAM workspace status sent to serial console.");break;
    case PSRE_M041_ACTION_EXPORT:
        ok=psre_m008_export_serial();set_status(ok?"Workspace export sent over serial HEX.":"Workspace export failed.");break;
    default:++state.failures;return false;
    }
    if(ok)++state.successes;else ++state.failures;
    return ok;
}
const psre_m041_status_t *psre_m041_get_status(void){return &state;}
static void serial_write_num(uint32_t value){
    char b[11];uint8_t n=0;
    do{b[n++]=(char)('0'+value%10u);value/=10u;}while(value&&n<sizeof(b));
    while(n)serial_write_char(b[--n]);
}
void psre_m041_report(void){
    serial_write("[M041] initialized=");serial_write(state.initialized?"yes":"no");
    serial_write(" mounted=");serial_write(state.mounted?"yes":"no");
    serial_write(" actions=");serial_write_num(state.actions);
    serial_write(" success=");serial_write_num(state.successes);
    serial_write(" failures=");serial_write_num(state.failures);
    serial_write("\n");
}
