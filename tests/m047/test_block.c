#include <psre/m047/block.h>
#include <psre/m046/bot.h>
#include <psre/m044/msc.h>
#include <assert.h>
#include <string.h>
static psre_m046_status_t bot={.initialized=true};
static uint32_t last_lba,last_blocks;
const psre_m046_status_t *psre_m046_get_status(void){return &bot;}
psre_m044_result_t psre_m046_scsi(psre_m044_scsi_op_t op,uint8_t lun,uint32_t lba,uint16_t blocks,void *data,uint32_t cap,uint32_t timeout,uint32_t *actual){
 (void)lun;(void)timeout;*actual=0;
 if(op==PSRE_M044_SCSI_INQUIRY){uint8_t *p=data;memset(p,' ',36);p[0]=0;p[8]='V';p[16]='D';*actual=36;return PSRE_M044_OK;}
 if(op==PSRE_M044_SCSI_TEST_UNIT_READY)return PSRE_M044_OK;
 if(op==PSRE_M044_SCSI_READ_CAPACITY10){uint8_t p[8]={0,0,0,255,0,0,2,0};assert(cap>=8);memcpy(data,p,8);*actual=8;return PSRE_M044_OK;}
 if(op==PSRE_M044_SCSI_READ10){last_lba=lba;last_blocks=blocks;assert(cap==(uint32_t)blocks*512);memset(data,0xA5,cap);*actual=cap;bot.last_residue=0;return PSRE_M044_OK;}
 return PSRE_M044_INVALID;
}
bool psre_m044_parse_read_capacity10(const uint8_t d[8],uint64_t *blocks,uint32_t *size){
 uint32_t last=((uint32_t)d[0]<<24)|((uint32_t)d[1]<<16)|((uint32_t)d[2]<<8)|d[3];
 *blocks=(uint64_t)last+1;*size=((uint32_t)d[4]<<24)|((uint32_t)d[5]<<16)|((uint32_t)d[6]<<8)|d[7];return *size==512;
}
void serial_write(const char *s){(void)s;} void serial_write_char(char c){(void)c;}
int main(void){
 assert(psre_m047_initialize(0));const psre_m047_status_t *s=psre_m047_get_status();
 assert(s->ready&&s->sectors==256&&s->block_size==512);
 uint8_t b[9*512];assert(psre_m047_read_sectors(7,9,b));
 assert(last_lba==15&&last_blocks==1);assert(b[0]==0xA5&&b[8*512]==0xA5);
 assert(!psre_m047_read_sectors(255,2,b));return 0;
}
