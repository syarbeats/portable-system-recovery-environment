#include <psre/m005/fat32.h>
#include <psre/m005/storage.h>
#include <psre/m047/block.h>
#include <psre/serial.h>
#include <stdint.h>
static uint16_t le16(const uint8_t*p){return (uint16_t)(p[0]|((uint16_t)p[1]<<8));}
static uint32_t le32(const uint8_t*p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
bool psre_fat32_probe(uint8_t dev,uint64_t lba,psre_fat32_volume_t*v){static uint8_t s[512] __attribute__((aligned(16)));if(!v||!psre_ata_read_sector(dev,lba,s))return false;const psre_ata_device_t *ds=psre_storage_get_devices();if(dev>=PSRE_STORAGE_MAX_DEVICES||lba>=ds[dev].sectors)return false;if(s[510]!=0x55||s[511]!=0xAA)return false;uint16_t bps=le16(&s[11]),reserved=le16(&s[14]),root_entries=le16(&s[17]),total16=le16(&s[19]),fat16=le16(&s[22]);uint8_t spc=s[13],fats=s[16];uint32_t total=le32(&s[32]),spf=le32(&s[36]),root=le32(&s[44]);if(bps!=512||!spc||(spc&(spc-1))||!reserved||!fats||root_entries||total16||fat16||!total||!spf||root<2||((uint64_t)lba+total>ds[dev].sectors))return false;v->valid=true;v->device_index=dev;v->partition_lba=lba;v->sectors_per_cluster=spc;v->reserved_sectors=reserved;v->fat_count=fats;v->sectors_per_fat=spf;v->root_cluster=root;v->total_sectors=total;return true;}
void psre_fat32_report(const psre_fat32_volume_t*v){if(!v||!v->valid)return;serial_write("[FAT32] device=");serial_write_char((char)('0'+v->device_index));serial_write(" start LBA=");char b[21];unsigned n=0;uint64_t x=v->partition_lba;if(!x)serial_write("0");else{while(x&&n<20){b[n++]=(char)('0'+x%10);x/=10;}while(n)serial_write_char(b[--n]);}serial_write(" sectors/cluster=");if(v->sectors_per_cluster<10)serial_write_char((char)('0'+v->sectors_per_cluster));else serial_write("multiple");serial_write(" root cluster=");if(v->root_cluster<10)serial_write_char((char)('0'+v->root_cluster));else serial_write("valid");serial_write("\n");}

bool psre_fat32_probe_usb(uint64_t lba,psre_fat32_volume_t *v){
 static uint8_t s[512] __attribute__((aligned(16)));
 const psre_m047_status_t *u=psre_m047_get_status();
 if(!v||!u||!u->ready||!psre_m047_read_sectors(lba,1,s))return false;
 if(s[510]!=0x55||s[511]!=0xAA)return false;
 uint16_t bps=le16(&s[11]),reserved=le16(&s[14]),root_entries=le16(&s[17]),total16=le16(&s[19]),fat16=le16(&s[22]);
 uint8_t spc=s[13],fats=s[16];uint32_t total=le32(&s[32]),spf=le32(&s[36]),root=le32(&s[44]);
 if(bps!=512||!spc||(spc&(spc-1))||!reserved||!fats||root_entries||total16||fat16||!total||!spf||root<2||(uint64_t)lba+total>u->sectors)return false;
 v->valid=true;v->device_index=0xFFu;v->partition_lba=lba;v->sectors_per_cluster=spc;v->reserved_sectors=reserved;
 v->fat_count=fats;v->sectors_per_fat=spf;v->root_cluster=root;v->total_sectors=total;return true;
}
