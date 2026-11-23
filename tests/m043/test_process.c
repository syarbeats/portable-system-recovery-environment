#include <psre/m043/process.h>
#include <psre/m006/fs.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint32_t qcount=2,qbytes=600,export_step,stage_calls;
static bool export_active;
void psre_m009_queue_manifest(void){}
bool psre_m009_queue_get_info(uint32_t*c,uint32_t*b){*c=qcount;*b=qbytes;return true;}
bool psre_m009_export_begin(void){export_active=true;export_step=0;return true;}
bool psre_m009_export_step(uint32_t budget,uint32_t*d,uint32_t*t,uint32_t*f,bool*done){assert(export_active&&budget>=32);export_step+=256;*d=export_step>qbytes?qbytes:export_step;*t=qbytes;*f=(*d>=256?1u:0u)+(*d>=512?1u:0u);*done=*d==qbytes;if(*done)export_active=false;return true;}
void psre_m009_export_cancel(void){export_active=false;}
bool psre_m008_recover(const char*p){assert(strcmp(p,"/A.TXT")==0);++stage_calls;return true;}
void serial_write(const char*s){(void)s;} void serial_write_char(char c){(void)c;}
int main(void){
 assert(psre_m043_initialize(PSRE_M038_INVALID_CONTAINER));
 assert(psre_m043_start_queue_export());
 assert(psre_m043_get_status()->state==PSRE_M043_RUNNING);
 assert(psre_m043_pause());psre_m043_tick();assert(export_step==0);
 assert(psre_m043_resume());psre_m043_tick();assert(export_step==256);
 psre_m043_tick();psre_m043_tick();
 assert(psre_m043_get_status()->state==PSRE_M043_COMPLETED);
 assert(psre_m043_get_status()->bytes_done==600);
 assert(psre_m043_get_status()->history_count==1);
 assert(psre_m043_start_stage("/A.TXT"));psre_m043_tick();
 assert(stage_calls==1&&psre_m043_get_status()->state==PSRE_M043_COMPLETED);
 assert(psre_m043_get_status()->history_count==2);
 puts("M043 Recovery Process Manager tests: PASS");return 0;
}
