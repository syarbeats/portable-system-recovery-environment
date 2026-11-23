#include <psre/m044/msc.h>
#include <psre/m045/bulk.h>
#include <psre/m019/usb_config.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/serial.h>
#include <stddef.h>
static psre_m044_status_t st;
static uint32_t next_tag=1u;
static uint32_t get_le32(const uint8_t *p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static uint32_t get_be32(const uint8_t *p){return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];}
bool psre_m044_find_interface(psre_m044_interface_t *out){
 if(!out) return false;
 *out=(psre_m044_interface_t){0};
 const psre_m019_status_t *cfg=psre_m019_get_status();
 if(!cfg||!cfg->success)return false;
 for(uint8_t i=0;i<cfg->interface_count;++i){const psre_m019_interface_t *it=&cfg->interfaces[i];
  if(it->class_code!=0x08u||it->subclass!=0x06u||it->protocol!=0x50u)continue;
  out->candidate=true;out->interface_supported=true;out->interface_number=it->number;out->subclass=it->subclass;out->protocol=it->protocol;
  for(uint8_t e=0;e<it->endpoint_count;++e){uint8_t ix=(uint8_t)(it->first_endpoint+e);if(ix>=cfg->endpoint_count)break;const psre_m019_endpoint_t *ep=&cfg->endpoints[ix];
   if((ep->attributes&3u)!=2u)continue;
   if(ep->address&0x80u){out->bulk_in=ep->address;out->bulk_in_packet=ep->max_packet_size;}
   else{out->bulk_out=ep->address;out->bulk_out_packet=ep->max_packet_size;}
  }
  out->endpoints_valid=out->bulk_in&&out->bulk_out&&out->bulk_in_packet&&out->bulk_out_packet;
  return out->endpoints_valid;
 }
 return false;
}
bool psre_m044_initialize(void){st=(psre_m044_status_t){0};st.initialized=true;psre_m044_interface_t it;
 if(!psre_m044_find_interface(&it)){serial_write("[M044] No supported MSC interface (08/06/50) with Bulk IN+OUT endpoints. Run M018, M019 and M020 first.\n");return false;}
 st.candidate_found=true;st.interface_number=it.interface_number;st.subclass=it.subclass;st.protocol=it.protocol;st.bulk_in=it.bulk_in;st.bulk_out=it.bulk_out;st.bulk_in_packet=it.bulk_in_packet;st.bulk_out_packet=it.bulk_out_packet;
 /* Endpoint configuration is an explicit operation via m044-activate. */
 serial_write("[M044] MSC BOT interface discovered; Bulk endpoints validated. Run m044-activate to configure endpoints with M045.\n");return true;
}
bool psre_m044_activate_transport(void){
 if(!st.initialized||!st.candidate_found)return false;
 if(!psre_m045_configure_bulk_endpoint(st.bulk_out,st.bulk_out_packet))return false;
 if(!psre_m045_configure_bulk_endpoint(st.bulk_in,st.bulk_in_packet))return false;
 st.transport_ready=true;serial_write("[M044] BOT bulk endpoints configured by M045.\n");return true;
}
bool psre_m044_build_cbw(psre_m044_cbw_t *out,uint8_t lun,uint32_t transfer_length,bool in,const uint8_t *cdb,uint8_t cdb_length){
 if(!out||!cdb||!cdb_length||cdb_length>16u||lun>=PSRE_M044_MAX_LUNS)return false;
 *out=(psre_m044_cbw_t){0};out->signature=PSRE_M044_BOT_CBW_SIGNATURE;out->tag=next_tag++;if(!next_tag)next_tag=1u;out->transfer_length=transfer_length;out->flags=in?0x80u:0u;out->lun=lun;out->command_length=cdb_length;for(uint8_t i=0;i<cdb_length;++i)out->command[i]=cdb[i];++st.commands_built;st.cbw_tag=out->tag;return true;
}
bool psre_m044_build_scsi(psre_m044_scsi_op_t op,uint8_t lun,uint32_t lba,uint16_t blocks,psre_m044_cbw_t *out){uint8_t cdb[16]={0};uint32_t bytes=0;bool in=false;uint8_t n=6;
 switch(op){case PSRE_M044_SCSI_TEST_UNIT_READY:cdb[0]=0x00u;break;
 case PSRE_M044_SCSI_INQUIRY:cdb[0]=0x12u;cdb[4]=36u;bytes=36u;in=true;break;
 case PSRE_M044_SCSI_REQUEST_SENSE:cdb[0]=0x03u;cdb[4]=18u;bytes=18u;in=true;break;
 case PSRE_M044_SCSI_READ_CAPACITY10:cdb[0]=0x25u;n=10u;bytes=8u;in=true;break;
 case PSRE_M044_SCSI_READ10:if(!blocks)return false;{uint32_t bs=st.last_block_size?st.last_block_size:512u;if((uint32_t)blocks>PSRE_M044_MAX_BLOCK_SIZE/bs)return false;bytes=(uint32_t)blocks*bs;}cdb[0]=0x28u;cdb[2]=(uint8_t)(lba>>24);cdb[3]=(uint8_t)(lba>>16);cdb[4]=(uint8_t)(lba>>8);cdb[5]=(uint8_t)lba;cdb[7]=(uint8_t)(blocks>>8);cdb[8]=(uint8_t)blocks;n=10u;in=true;break;
 default:return false;}
 return psre_m044_build_cbw(out,lun,bytes,in,cdb,n);
}
psre_m044_result_t psre_m044_validate_csw(const uint8_t *data,uint32_t length,uint32_t tag,uint32_t expected,uint32_t *residue){if(!data||length!=PSRE_M044_CSW_LENGTH)return PSRE_M044_INVALID;
 uint32_t sig=get_le32(data),got=get_le32(data+4),r=get_le32(data+8);if(sig!=PSRE_M044_BOT_CSW_SIGNATURE||got!=tag||r>expected){++st.protocol_errors;return PSRE_M044_PROTOCOL_ERROR;}if(residue)*residue=r;if(data[12]>2u){++st.protocol_errors;return PSRE_M044_PROTOCOL_ERROR;}++st.csw_validated;return data[12]==0u?PSRE_M044_OK:(data[12]==1u?PSRE_M044_FAILED:PSRE_M044_PROTOCOL_ERROR);}
bool psre_m044_parse_read_capacity10(const uint8_t data[8],uint64_t *blocks,uint32_t *size){if(!data||!blocks||!size)return false;uint32_t last=get_be32(data),bs=get_be32(data+4);if(bs<512u||bs>PSRE_M044_MAX_BLOCK_SIZE||(bs&(bs-1u)))return false;*blocks=(uint64_t)last+1u;*size=bs;st.last_capacity_blocks=*blocks;st.last_block_size=bs;return true;}
const psre_m044_status_t *psre_m044_get_status(void){return &st;}
void psre_m044_report(void){serial_write("\n--- PSRE M044 USB MASS STORAGE ---\nMSC candidate=");serial_write(st.candidate_found?"yes":"no");serial_write(st.transport_ready?" BOT transport=ready\nBulk IN=0x":" BOT transport=not-active\nBulk IN=0x");static const char h[]="0123456789ABCDEF";serial_write_char(h[st.bulk_in>>4]);serial_write_char(h[st.bulk_in&15]);serial_write(" Bulk OUT=0x");serial_write_char(h[st.bulk_out>>4]);serial_write_char(h[st.bulk_out&15]);serial_write(" CBW built=");serial_write(st.commands_built?"yes":"no");serial_write("\n");}
