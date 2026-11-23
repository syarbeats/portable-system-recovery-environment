#include <psre/m004/pci.h>
#include <psre/serial.h>
#include <stdint.h>
#define PCI_ADDR 0xCF8
#define PCI_DATA 0xCFC
#define PCI_NONE 0xFFFF
static psre_pci_device_t devices[PSRE_PCI_MAX_DEVICES];
static psre_pci_stats_t stats;
static inline void outl(uint16_t p,uint32_t v){__asm__ volatile("outl %0,%1"::"a"(v),"Nd"(p));}
static inline uint32_t inl(uint16_t p){uint32_t v;__asm__ volatile("inl %1,%0":"=a"(v):"Nd"(p));return v;}
static inline uint16_t inw(uint16_t p){uint16_t v;__asm__ volatile("inw %1,%0":"=a"(v):"Nd"(p));return v;}
static inline uint8_t inb(uint16_t p){uint8_t v;__asm__ volatile("inb %1,%0":"=a"(v):"Nd"(p));return v;}
static inline void outw(uint16_t p,uint16_t v){__asm__ volatile("outw %0,%1"::"a"(v),"Nd"(p));}
static inline void outb(uint16_t p,uint8_t v){__asm__ volatile("outb %0,%1"::"a"(v),"Nd"(p));}
static void cfg_select(uint8_t b,uint8_t d,uint8_t f,uint8_t o){
    uint32_t a=0x80000000u|((uint32_t)b<<16)|((uint32_t)d<<11)|
               ((uint32_t)f<<8)|(o&0xFCu);
    outl(PCI_ADDR,a);
}
static uint32_t cfg(uint8_t b,uint8_t d,uint8_t f,uint8_t o){uint32_t a=0x80000000u|((uint32_t)b<<16)|((uint32_t)d<<11)|((uint32_t)f<<8)|(o&0xFCu),v;__asm__ volatile("pushfq; cli":::"memory");outl(PCI_ADDR,a);v=inl(PCI_DATA);__asm__ volatile("popfq":::"memory","cc");return v;}
static uint16_t rd16(uint8_t b,uint8_t d,uint8_t f,uint8_t o){return (uint16_t)(cfg(b,d,f,o)>>((o&2u)*8u));}
static uint8_t rd8(uint8_t b,uint8_t d,uint8_t f,uint8_t o){return (uint8_t)(cfg(b,d,f,o)>>((o&3u)*8u));}
const char *psre_pci_class_name(uint8_t c,uint8_t s){
 if(c==1){if(s==1)return "IDE controller";if(s==6)return "SATA controller";if(s==8)return "NVMe/non-volatile memory";return "Mass storage";}
 if(c==2)return "Network controller";
 if(c==3)return "Display controller";
 if(c==4)return "Multimedia controller";
 if(c==5)return "Memory controller";
 if(c==6){if(s==0)return "Host bridge";if(s==1)return "ISA bridge";if(s==4)return "PCI-to-PCI bridge";return "Bridge";}
 if(c==0x0C&&s==3)return "USB controller";
 if(c==0x0C&&s==5)return "SMBus controller";
 if(c==0x0B)return "Processor";
 return "Other/unknown";
}
static void bars_read(psre_pci_device_t *d){uint8_t h=d->header_type&0x7F,n=h==0?6:(h==1?2:0);for(uint8_t i=0;i<n&&i<6;i++){uint8_t off=(uint8_t)(0x10+i*4);uint32_t lo=cfg(d->bus,d->device,d->function,off);if(!lo||lo==0xFFFFFFFFu)continue;psre_pci_bar_t *b=&d->bars[d->bar_count++];b->index=i;if(lo&1){b->kind=1;b->address=lo&~3u;}else{uint8_t t=(lo>>1)&3;b->kind=2;b->is_64bit=(t==2);b->prefetchable=(lo>>3)&1;b->address=lo&~0xFu;if(b->is_64bit&&i+1<n){b->address|=(uint64_t)cfg(d->bus,d->device,d->function,off+4)<<32;++i;}}}}
static void store(uint8_t b,uint8_t d,uint8_t f){uint32_t id=cfg(b,d,f,0);if((uint16_t)id==PCI_NONE)return;++stats.discovered;if(stats.stored>=PSRE_PCI_MAX_DEVICES)return;psre_pci_device_t *x=&devices[stats.stored++];x->bus=b;x->device=d;x->function=f;x->vendor_id=(uint16_t)id;x->device_id=(uint16_t)(id>>16);uint32_t cs=cfg(b,d,f,4),cr=cfg(b,d,f,8);x->command=(uint16_t)cs;x->status=cs>>16;x->revision=cr;x->prog_if=cr>>8;x->subclass=cr>>16;x->class_code=cr>>24;x->header_type=rd8(b,d,f,0x0E);x->irq_line=rd8(b,d,f,0x3C);x->irq_pin=rd8(b,d,f,0x3D);if((x->header_type&0x7F)==1&&x->class_code==6&&x->subclass==4)++stats.bridges;bars_read(x);}
bool psre_pci_initialize(void){for(uint32_t i=0;i<sizeof(devices);i++)((uint8_t*)devices)[i]=0;for(uint32_t i=0;i<sizeof(stats);i++)((uint8_t*)&stats)[i]=0;stats.config_mechanism_1_available=(cfg(0,0,0,0)!=0xFFFFFFFFu);if(!stats.config_mechanism_1_available)return false;
 for(uint32_t b=0;b<256;b++) {
  for(uint32_t d=0;d<32;d++) {
   if(rd16((uint8_t)b,(uint8_t)d,0,0)==PCI_NONE) continue;
   uint8_t h=rd8((uint8_t)b,(uint8_t)d,0,0x0E),n=(h&0x80)?8:1;
   if(n>1) ++stats.multifunction_devices;
   for(uint8_t f=0;f<n;f++) store((uint8_t)b,(uint8_t)d,f);
  }
 }
 return true;
}
const psre_pci_device_t *psre_pci_get_devices(void){return devices;}const psre_pci_stats_t *psre_pci_get_stats(void){return &stats;}
static void hex(uint32_t v,unsigned n){static const char h[]="0123456789ABCDEF";char b[9];for(unsigned i=0;i<n;i++)b[i]=h[(v>>((n-1-i)*4))&15];b[n]=0;serial_write(b);}
void psre_pci_report(void){serial_write("\n--- PSRE PCI INVENTORY ---\n");for(uint32_t i=0;i<stats.stored;i++){const psre_pci_device_t*d=&devices[i];serial_write("[PCI ");hex(d->bus,2);serial_write(":");hex(d->device,2);serial_write(".");hex(d->function,1);serial_write("] ");hex(d->vendor_id,4);serial_write(":");hex(d->device_id,4);serial_write(" | ");serial_write(psre_pci_class_name(d->class_code,d->subclass));serial_write(" | BARs=");hex(d->bar_count,1);serial_write("\n");}serial_write("[M004] discovered=");hex(stats.discovered,8);serial_write(" stored=");hex(stats.stored,8);serial_write(" bridges=");hex(stats.bridges,8);serial_write("\n--------------------------\n");}



/* M013: bounded PCI configuration-space accessors (mechanism #1). */
static bool pci_bdf_valid(uint8_t device, uint8_t function, uint8_t offset) {
    return device < 32 && function < 8 && offset <= 0xFC && (offset & 3u) == 0;
}
bool psre_pci_config_read32(uint8_t bus, uint8_t device, uint8_t function,
                            uint8_t offset, uint32_t *value) {
    if (!value || !stats.config_mechanism_1_available ||
        !pci_bdf_valid(device,function,offset)) return false;
    *value=cfg(bus,device,function,offset);
    return *value != 0xFFFFFFFFu;
}
bool psre_pci_config_write32(uint8_t bus, uint8_t device, uint8_t function,
                             uint8_t offset, uint32_t value) {
    if (!stats.config_mechanism_1_available ||
        !pci_bdf_valid(device,function,offset)) return false;
    if (cfg(bus,device,function,0)==0xFFFFFFFFu) return false;
    uint32_t address=0x80000000u|((uint32_t)bus<<16)|
        ((uint32_t)device<<11)|((uint32_t)function<<8)|(offset&0xFCu);
    __asm__ volatile ("pushfq; cli" ::: "memory");
    outl(PCI_ADDR,address);
    outl(PCI_DATA,value);
    __asm__ volatile ("popfq" ::: "memory","cc");
    return true;
}
bool psre_pci_config_read16(uint8_t bus,uint8_t device,uint8_t function,
                            uint8_t offset,uint16_t *value) {
    if(!value || offset>0xFE || !stats.config_mechanism_1_available ||
       device>=32 || function>=8 || cfg(bus,device,function,0)==0xFFFFFFFFu)return false;
    __asm__ volatile ("pushfq; cli" ::: "memory");
    cfg_select(bus,device,function,offset);
    *value=inw((uint16_t)(PCI_DATA+(offset&2u)));
    __asm__ volatile ("popfq" ::: "memory","cc");
    return true;
}
bool psre_pci_config_write16(uint8_t bus,uint8_t device,uint8_t function,
                             uint8_t offset,uint16_t value) {
    if(offset>0xFE || !stats.config_mechanism_1_available ||
       device>=32 || function>=8 || cfg(bus,device,function,0)==0xFFFFFFFFu)return false;
    __asm__ volatile ("pushfq; cli" ::: "memory");
    cfg_select(bus,device,function,offset);
    outw((uint16_t)(PCI_DATA+(offset&2u)),value);
    __asm__ volatile ("popfq" ::: "memory","cc");
    return true;
}
bool psre_pci_config_read8(uint8_t bus,uint8_t device,uint8_t function,
                           uint8_t offset,uint8_t *value) {
    if(!value || !stats.config_mechanism_1_available ||
       device>=32 || function>=8 || cfg(bus,device,function,0)==0xFFFFFFFFu)return false;
    __asm__ volatile ("pushfq; cli" ::: "memory");
    cfg_select(bus,device,function,offset);
    *value=inb((uint16_t)(PCI_DATA+(offset&3u)));
    __asm__ volatile ("popfq" ::: "memory","cc");
    return true;
}
bool psre_pci_config_write8(uint8_t bus,uint8_t device,uint8_t function,
                            uint8_t offset,uint8_t value) {
    if(!stats.config_mechanism_1_available ||
       device>=32 || function>=8 || cfg(bus,device,function,0)==0xFFFFFFFFu)return false;
    __asm__ volatile ("pushfq; cli" ::: "memory");
    cfg_select(bus,device,function,offset);
    outb((uint16_t)(PCI_DATA+(offset&3u)),value);
    __asm__ volatile ("popfq" ::: "memory","cc");
    return true;
}
bool psre_pci_enable_mmio_busmaster(uint8_t bus,uint8_t device,uint8_t function) {
    bool found=false,has_mmio_bar=false;
    for(uint32_t i=0;i<stats.stored;i++) {
        const psre_pci_device_t *d=&devices[i];
        if(d->bus==bus && d->device==device && d->function==function) {
            found=true;
            for(uint8_t b=0;b<d->bar_count;b++)
                if(d->bars[b].kind==2 && d->bars[b].address) has_mmio_bar=true;
            break;
        }
    }
    if(!found || !has_mmio_bar)return false;
    uint16_t command;
    if(!psre_pci_config_read16(bus,device,function,0x04,&command))return false;
    /* Enable Memory Space + Bus Master. Never called automatically by M013. */
    command=(uint16_t)(command | (1u<<1) | (1u<<2));
    return psre_pci_config_write16(bus,device,function,0x04,command);
}
