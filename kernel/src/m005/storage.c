#include <psre/m005/storage.h>
#include <psre/serial.h>
#include <stdint.h>
#include <stddef.h>
#define R_DATA 0
#define R_FEATURES 1
#define R_COUNT 2
#define R_LBA0 3
#define R_LBA1 4
#define R_LBA2 5
#define R_DRIVE 6
#define R_STATUS 7
#define R_COMMAND 7
#define S_BSY 0x80
#define S_DRQ 0x08
#define S_ERR 0x01
#define S_DF 0x20
#define CMD_IDENTIFY 0xEC
#define CMD_READ 0x20
static const uint16_t bases[2]={0x1F0,0x170}, ctrls[2]={0x3F6,0x376};
static psre_ata_device_t devices[4];
static psre_storage_stats_t stats;
static inline void outb(uint16_t p,uint8_t v){__asm__ volatile("outb %0,%1"::"a"(v),"Nd"(p));}
static inline uint8_t inb(uint16_t p){uint8_t v;__asm__ volatile("inb %1,%0":"=a"(v):"Nd"(p));return v;}
static inline uint16_t inw(uint16_t p){uint16_t v;__asm__ volatile("inw %1,%0":"=a"(v):"Nd"(p));return v;}
static void delay400(uint16_t c){(void)inb(c);(void)inb(c);(void)inb(c);(void)inb(c);}
static bool wait_idle(uint16_t b,uint16_t c){for(uint32_t i=0;i<1000000;i++){uint8_t s=inb(b+R_STATUS);if(!s||s==0xFF)return false;if(!(s&S_BSY))return true;(void)inb(c);}return false;}
static bool wait_data(uint16_t b,uint16_t c){for(uint32_t i=0;i<1000000;i++){uint8_t s=inb(b+R_STATUS);if(!s||s==0xFF)return false;if(s&S_BSY){(void)inb(c);continue;}if(s&(S_ERR|S_DF))return false;if(s&S_DRQ)return true;(void)inb(c);}return false;}
static void ident_string(char *dst,const uint16_t *src){for(unsigned i=0;i<20;i++){dst[2*i]=(char)(src[i]>>8);dst[2*i+1]=(char)src[i];}dst[40]=0;for(int i=39;i>=0&&dst[i]==' ';i--)dst[i]=0;}
static bool identify(uint8_t ch,uint8_t dr,psre_ata_device_t *d){uint16_t b=bases[ch],c=ctrls[ch];outb(c,2);outb(b+R_DRIVE,(uint8_t)(0xA0|(dr<<4)));delay400(c);outb(b+R_COUNT,0);outb(b+R_LBA0,0);outb(b+R_LBA1,0);outb(b+R_LBA2,0);outb(b+R_COMMAND,CMD_IDENTIFY);delay400(c);if(inb(b+R_STATUS)==0||!wait_idle(b,c))return false;if(inb(b+R_LBA1)||inb(b+R_LBA2)||!wait_data(b,c))return false;uint16_t id[256];for(unsigned i=0;i<256;i++)id[i]=inw(b+R_DATA);d->present=true;d->channel=ch;d->drive=dr;d->signature=id[0];d->capabilities=id[49];d->command_sets=((uint32_t)id[83]<<16)|id[82];d->sectors=((uint64_t)id[103]<<48)|((uint64_t)id[102]<<32)|((uint64_t)id[101]<<16)|id[100];if(!(id[83]&(1u<<10))||!d->sectors)d->sectors=((uint64_t)id[61]<<16)|id[60];ident_string(d->model,&id[27]);if(!d->sectors){d->present=false;return false;}++stats.detected_devices;return true;}
bool psre_storage_detect_all(void){for(unsigned i=0;i<sizeof(devices);i++)((uint8_t*)devices)[i]=0;for(unsigned i=0;i<sizeof(stats);i++)((uint8_t*)&stats)[i]=0;for(uint8_t ch=0;ch<2;ch++)for(uint8_t dr=0;dr<2;dr++)(void)identify(ch,dr,&devices[ch*2+dr]);return stats.detected_devices!=0;}
bool psre_ata_read_sector(uint8_t ix,uint64_t lba,void *buf){
    if(ix>=4||!buf||!devices[ix].present||lba>=devices[ix].sectors||lba>0x0FFFFFFFULL)return false;
    uint8_t ch=devices[ix].channel,dr=devices[ix].drive;
    uint16_t b=bases[ch],c=ctrls[ch];
    outb(b+R_DRIVE,(uint8_t)(0xE0|(dr<<4)|((lba>>24)&15)));
    delay400(c);
    if(!wait_idle(b,c))return false;
    outb(b+R_FEATURES,0);
    outb(b+R_COUNT,1);
    outb(b+R_LBA0,(uint8_t)lba);
    outb(b+R_LBA1,(uint8_t)(lba>>8));
    outb(b+R_LBA2,(uint8_t)(lba>>16));
    outb(b+R_COMMAND,CMD_READ);
    if(!wait_data(b,c))return false;
    uint16_t *w=(uint16_t*)buf;
    for(unsigned i=0;i<256;i++)w[i]=inw(b+R_DATA);
    (void)inb(b+R_STATUS);
    return true;
}
const psre_ata_device_t *psre_storage_get_devices(void){return devices;}
const psre_storage_stats_t *psre_storage_get_stats(void){return &stats;}
void psre_storage_record_readable(void){++stats.readable_devices;}
void psre_storage_record_fat32(void){++stats.fat32_volumes;}
static void dec(uint64_t x){char b[21];unsigned n=0;if(!x){serial_write("0");return;}while(x&&n<20){b[n++]=(char)('0'+x%10);x/=10;}while(n)serial_write_char(b[--n]);}
void psre_storage_report(void){serial_write("\n--- PSRE ATA STORAGE ---\n");for(uint8_t i=0;i<4;i++)if(devices[i].present){serial_write("[ATA] device=");dec(i);serial_write(" model=");serial_write(devices[i].model);serial_write(" sectors=");dec(devices[i].sectors);serial_write(" capacity MiB=");dec(devices[i].sectors/2048);serial_write("\n");}serial_write("[M005] detected=");dec(stats.detected_devices);serial_write(" readable=");dec(stats.readable_devices);serial_write(" FAT32=");dec(stats.fat32_volumes);serial_write("\n------------------------\n");}
