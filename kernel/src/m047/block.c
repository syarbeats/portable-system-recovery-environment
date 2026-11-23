#include <psre/m047/block.h>
#include <psre/m046/bot.h>
#include <psre/m044/msc.h>
#include <psre/serial.h>
static psre_m047_status_t st;
static uint8_t scratch[PSRE_M044_MAX_BLOCK_SIZE] __attribute__((aligned(64)));
static void copy_ascii(char *d,const uint8_t *s,unsigned n){
 for(unsigned i=0;i<n;i++){uint8_t c=s[i];d[i]=(c>=32&&c<127)?(char)c:' ';} d[n]=0;
}
static void print_u64(uint64_t v){
 char b[21];unsigned n=0;do{b[n++]=(char)('0'+v%10u);v/=10u;}while(v&&n<20);while(n)serial_write_char(b[--n]);
}
bool psre_m047_initialize(uint8_t lun){
 st=(psre_m047_status_t){0};
 const psre_m046_status_t *bot=psre_m046_get_status();
 if(!bot||!bot->initialized||bot->reset_required){serial_write("[M047] BOT is not initialized or requires reset recovery.\n");return false;}
 uint32_t actual=0;
 if(psre_m046_scsi(PSRE_M044_SCSI_INQUIRY,lun,0,0,scratch,36,1000,&actual)!=PSRE_M044_OK||actual!=36){++st.errors;return false;}
 st.lun=lun;st.peripheral_type=scratch[0]&31u;st.removable=(scratch[1]>>7)&1u;
 copy_ascii(st.vendor,&scratch[8],8);copy_ascii(st.product,&scratch[16],16);copy_ascii(st.revision,&scratch[32],4);
 if(psre_m046_scsi(PSRE_M044_SCSI_TEST_UNIT_READY,lun,0,0,0,0,1000,&actual)!=PSRE_M044_OK){++st.errors;return false;}
 if(psre_m046_scsi(PSRE_M044_SCSI_READ_CAPACITY10,lun,0,0,scratch,8,1000,&actual)!=PSRE_M044_OK||actual!=8||
    !psre_m044_parse_read_capacity10(scratch,&st.sectors,&st.block_size)||st.block_size!=PSRE_M047_SECTOR_SIZE){++st.errors;return false;}
 st.initialized=true;st.present=true;st.ready=true;
 serial_write("[M047] USB block device ready. Sectors=");print_u64(st.sectors);serial_write(" block-size=512\n");
 return true;
}
bool psre_m047_read_sectors(uint64_t lba,uint32_t count,void *buffer){
 if(!st.ready||!buffer||!count||st.block_size!=512u||lba>=st.sectors||count>st.sectors-lba){++st.errors;return false;}
 uint8_t *dst=(uint8_t*)buffer;uint32_t remaining=count;
 while(remaining){
  uint16_t chunk=(uint16_t)(remaining>PSRE_M047_MAX_READ_SECTORS?PSRE_M047_MAX_READ_SECTORS:remaining);
  uint32_t bytes=(uint32_t)chunk*512u,actual=0;
  if(lba>0xFFFFFFFFull||bytes>sizeof(scratch)){++st.errors;return false;}
  psre_m044_result_t r=psre_m046_scsi(PSRE_M044_SCSI_READ10,st.lun,(uint32_t)lba,chunk,scratch,bytes,2000,&actual);
  if(r!=PSRE_M044_OK||actual!=bytes||psre_m046_get_status()->last_residue!=0){++st.errors;st.ready=false;return false;}
  for(uint32_t i=0;i<bytes;i++)dst[i]=scratch[i];
  dst+=bytes;lba+=chunk;remaining-=chunk;++st.read_commands;st.sectors_read+=chunk;
 }
 return true;
}
const psre_m047_status_t *psre_m047_get_status(void){return &st;}
void psre_m047_report(void){
 serial_write("\n--- PSRE M047 USB BLOCK DEVICE ---\n");serial_write(st.ready?"Ready=yes\n":"Ready=no\n");
 serial_write("Vendor: ");serial_write(st.vendor);serial_write("\nProduct: ");serial_write(st.product);
 serial_write("\nRevision: ");serial_write(st.revision);serial_write("\nBlock size: ");print_u64(st.block_size);
 serial_write("\nSectors: ");print_u64(st.sectors);serial_write("\nRead commands: ");print_u64(st.read_commands);
 serial_write("\nSectors read: ");print_u64(st.sectors_read);serial_write("\nErrors: ");print_u64(st.errors);serial_write("\n");
}
