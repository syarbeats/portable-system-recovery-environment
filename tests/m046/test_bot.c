#include <psre/m046/bot.h>
#include <psre/m045/bulk.h>
#include <assert.h>
#include <string.h>
static uint8_t out_data[64];
static unsigned call_no;
void serial_write(const char *s){(void)s;}
void serial_write_char(char c){(void)c;}
static psre_m044_status_t mstat={.initialized=true,.candidate_found=true,.transport_ready=true,.bulk_in=0x81,.bulk_out=0x02};
const psre_m044_status_t *psre_m044_get_status(void){return &mstat;}
bool psre_m044_build_scsi(psre_m044_scsi_op_t op,uint8_t lun,uint32_t lba,uint16_t blocks,psre_m044_cbw_t *o){
 (void)lun;(void)lba;(void)blocks;memset(o,0,sizeof(*o));o->signature=PSRE_M044_BOT_CBW_SIGNATURE;o->tag=0x1234;o->command_length=6;
 if(op==PSRE_M044_SCSI_INQUIRY){o->transfer_length=36;o->flags=0x80;}return true;
}
psre_m044_result_t psre_m044_validate_csw(const uint8_t *d,uint32_t n,uint32_t tag,uint32_t exp,uint32_t *r){
 (void)exp;assert(n==13);assert(d[12]==0);assert(tag==0x1234);if(r)*r=0;return PSRE_M044_OK;
}
bool psre_m045_bulk_transfer(uint8_t ep,void *buf,uint32_t len,uint32_t timeout,uint32_t *actual){
 (void)timeout; ++call_no;
 if(ep==0x02 && len==31){*actual=len;return true;}
 if(ep==0x81 && len==13){uint8_t *p=buf;memset(p,0,13);p[12]=0;*actual=13;return true;}
 if(ep==0x81){memset(buf,0x5a,len);*actual=len;return true;}
 if(ep==0x02){memcpy(out_data,buf,len);*actual=len;return true;}
 return false;
}
int main(void){
 assert(psre_m046_initialize());
 uint8_t data[36];uint32_t actual=0;
 assert(psre_m046_scsi(PSRE_M044_SCSI_INQUIRY,0,0,0,data,sizeof(data),100,&actual)==PSRE_M044_OK);
 assert(actual==36 && data[0]==0x5a);
 assert(psre_m046_get_status()->commands==1);
 assert(psre_m046_get_status()->data_bytes_in==36);
 assert(!psre_m046_reset_recovery());
 return 0;
}
