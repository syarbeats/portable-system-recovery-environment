#include <psre/m041/recovery_manager.h>
#include <psre/m043/process.h>
#include <psre/m006/fs.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
static char widget_text[16][128];
static psre_m043_status_t m043_status;
static int mount_calls,list_calls,stat_calls,crc_calls,recover_calls,stage_job_calls,queue_add_calls,queue_export_calls,queue_clear_calls,manifest_calls,workspace_calls,export_calls;
bool psre_m038_get_container(psre_m038_container_id_t id,psre_m038_container_info_t*out){(void)id;*out=(psre_m038_container_info_t){.used=true};return true;}
psre_m038_widget_id_t psre_m038_add_textbox(psre_m038_container_id_t c,const char*t,int32_t x,int32_t y,int32_t w,int32_t h,uint16_t m){(void)c;(void)x;(void)y;(void)w;(void)h;(void)m;strncpy(widget_text[0],t,127);return 0;}
psre_m038_widget_id_t psre_m038_add_label(psre_m038_container_id_t c,const char*t,int32_t x,int32_t y,uint32_t w){(void)c;(void)t;(void)x;(void)y;(void)w;return 1;}
psre_m038_widget_id_t psre_m038_add_button(psre_m038_container_id_t c,const char*t,int32_t x,int32_t y,int32_t w,int32_t h,psre_m038_action_t a,void*ctx){(void)c;(void)t;(void)x;(void)y;(void)w;(void)h;(void)a;(void)ctx;static uint8_t id=2;return id++;}
bool psre_m038_set_text(psre_m038_container_id_t c,psre_m038_widget_id_t w,const char*t){(void)c;strncpy(widget_text[w],t,127);return true;}
bool psre_m038_get_text(psre_m038_container_id_t c,psre_m038_widget_id_t w,char*out,uint32_t n){(void)c;if(!n)return false;strncpy(out,widget_text[w],n-1);out[n-1]=0;return true;}
bool psre_fat32_is_mounted(void){return mount_calls>0;}
bool psre_fat32_mount_first(void){++mount_calls;return true;}
bool psre_m007_list_dir(const char*p){(void)p;++list_calls;return true;}
bool psre_m007_stat(const char*p){(void)p;++stat_calls;return true;}
bool psre_m007_crc32(const char*p){(void)p;++crc_calls;return true;}
bool psre_m008_recover(const char*p){(void)p;++recover_calls;return true;}
bool psre_m009_queue_add(const char*p){(void)p;++queue_add_calls;return true;}
void psre_m009_queue_list(void){}
bool psre_m009_queue_export(void){++queue_export_calls;return true;}
bool psre_m043_initialize(psre_m038_container_id_t c){(void)c;m043_status.initialized=true;return true;}
bool psre_m043_start_queue_export(void){++queue_export_calls;m043_status.state=PSRE_M043_RUNNING;return true;}
bool psre_m043_start_stage(const char*p){(void)p;++stage_job_calls;return true;}
const psre_m043_status_t*psre_m043_get_status(void){return &m043_status;}
bool psre_m043_pause(void){m043_status.state=PSRE_M043_PAUSED;return true;}
bool psre_m043_resume(void){m043_status.state=PSRE_M043_RUNNING;return true;}
bool psre_m043_retry(void){return true;}
void psre_m009_queue_clear(void){++queue_clear_calls;}
void psre_m009_queue_manifest(void){++manifest_calls;}
void psre_m008_manifest(void){++manifest_calls;}
void psre_m008_workspace_info(void){++workspace_calls;}
bool psre_m008_export_serial(void){++export_calls;return true;}
void serial_write(const char*s){(void)s;}
void serial_write_char(char c){(void)c;}
int main(void){
 assert(psre_m041_initialize(0));
 const psre_m041_status_t *st=psre_m041_get_status();
 assert(st->initialized && st->container==0);
 assert(psre_m041_set_path("/BACKUP.BIN"));
 assert(psre_m041_execute(PSRE_M041_ACTION_MOUNT));
 assert(psre_m041_execute(PSRE_M041_ACTION_LIST));
 assert(psre_m041_execute(PSRE_M041_ACTION_STAT));
 assert(psre_m041_execute(PSRE_M041_ACTION_CRC32));
 assert(psre_m041_execute(PSRE_M041_ACTION_RECOVER));
 assert(psre_m041_execute(PSRE_M041_ACTION_QUEUE_ADD));
 assert(psre_m041_execute(PSRE_M041_ACTION_QUEUE_EXPORT));
 assert(psre_m041_execute(PSRE_M041_ACTION_QUEUE_CLEAR));
 assert(psre_m041_execute(PSRE_M041_ACTION_MANIFEST));
 assert(psre_m041_execute(PSRE_M041_ACTION_WORKSPACE));
 assert(psre_m041_execute(PSRE_M041_ACTION_EXPORT));
 assert(mount_calls==1&&list_calls==1&&stat_calls==1&&crc_calls==1);
 assert(recover_calls==0&&stage_job_calls==1&&queue_add_calls==1&&queue_export_calls==1);
 assert(queue_clear_calls==1&&manifest_calls==2&&workspace_calls==1&&export_calls==1);
 assert(st->actions==11&&st->successes==11&&st->failures==0);
 assert(!psre_m041_set_path("")); /* empty path is rejected */
 puts("M041 Recovery Manager tests: PASS");
 return 0;
}
