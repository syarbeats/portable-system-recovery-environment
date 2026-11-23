#include <psre/m044/msc.h>
#include <psre/m019/usb_config.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
const psre_m019_status_t *psre_m019_get_status(void){return 0;}
bool psre_m045_configure_bulk_endpoint(uint8_t address,uint16_t mps){(void)address;(void)mps;return true;}
void serial_write(const char *s){(void)s;}
void serial_write_char(char c){(void)c;}
static void le32(uint8_t*p,uint32_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);}
int main(void){psre_m044_cbw_t c;assert(psre_m044_build_scsi(PSRE_M044_SCSI_INQUIRY,0,0,0,&c));assert(c.signature==PSRE_M044_BOT_CBW_SIGNATURE&&c.transfer_length==36&&c.flags==0x80&&c.command[0]==0x12);
 assert(psre_m044_build_scsi(PSRE_M044_SCSI_READ10,0,0x12345678,2,&c));assert(c.transfer_length==1024&&c.command_length==10&&c.command[0]==0x28&&c.command[2]==0x12&&c.command[5]==0x78&&c.command[8]==2);
 uint8_t csw[13]={0};le32(csw,PSRE_M044_BOT_CSW_SIGNATURE);le32(csw+4,c.tag);le32(csw+8,0);uint32_t residue=99;assert(psre_m044_validate_csw(csw,13,c.tag,1024,&residue)==PSRE_M044_OK&&residue==0);
 le32(csw+4,c.tag+1);assert(psre_m044_validate_csw(csw,13,c.tag,1024,0)==PSRE_M044_PROTOCOL_ERROR);
 uint8_t cap[8]={0,0,0,15,0,0,2,0};uint64_t blocks;uint32_t size;assert(psre_m044_parse_read_capacity10(cap,&blocks,&size)&&blocks==16&&size==512);
 cap[7]=1;assert(!psre_m044_parse_read_capacity10(cap,&blocks,&size));puts("M044 MSC protocol tests: PASS");return 0;}
