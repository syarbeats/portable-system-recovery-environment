#include <psre/m005/m005.h>
#include <psre/m005/storage.h>
#include <psre/m005/fat32.h>
#include <psre/serial.h>
#include <stdint.h>
static uint32_t le32(const uint8_t*p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void probe(uint8_t dev){uint8_t s[512] __attribute__((aligned(16)));if(!psre_ata_read_sector(dev,0,s)){serial_write("[M005] Sector-zero read failed\n");return;}psre_storage_record_readable();psre_fat32_volume_t v={0};if(psre_fat32_probe(dev,0,&v)){psre_storage_record_fat32();psre_fat32_report(&v);return;}if(s[510]!=0x55||s[511]!=0xAA)return;for(unsigned p=0;p<4;p++){const uint8_t*e=&s[446+p*16];uint8_t t=e[4];uint32_t start=le32(&e[8]);if(!start||!(t==0x0B||t==0x0C||t==0x1B||t==0x1C))continue;if(psre_fat32_probe(dev,start,&v)){psre_storage_record_fat32();psre_fat32_report(&v);}else serial_write("[M005] FAT32 partition entry found but BPB invalid\n");}}
void psre_m005_initialize(void){serial_write("\n=== PSRE M005 STORAGE SUBSYSTEM ===\n");if(!psre_storage_detect_all()){serial_write("[WARN] No ATA PIO disks detected. AHCI/NVMe/ATAPI are not implemented yet.\n");psre_storage_report();return;}const psre_ata_device_t*d=psre_storage_get_devices();for(uint8_t i=0;i<4;i++)if(d[i].present)probe(i);psre_storage_report();serial_write("[M005] Read-only storage discovery complete\n");}
