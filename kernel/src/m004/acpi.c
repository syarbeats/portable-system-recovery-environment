#include <psre/m004/acpi.h>
#include <psre/limine.h>
#include <psre/memory_map.h>
#include <psre/serial.h>
#include <stdint.h>
#include <stddef.h>
extern volatile struct limine_rsdp_request psre_rsdp_request;
typedef struct __attribute__((packed)){char sig[8];uint8_t checksum;char oem[6];uint8_t revision;uint32_t rsdt;uint32_t length;uint64_t xsdt;uint8_t ext_checksum;uint8_t reserved[3];} rsdp_t;
typedef struct __attribute__((packed)){psre_acpi_sdt_header_t h;uint32_t lapic_addr;uint32_t flags;} madt_t;
typedef struct __attribute__((packed)){uint8_t type,length;} madt_entry_t;
static psre_acpi_stats_t stats;static uint64_t hhdm;
static uint8_t sum8(const void*p,uint32_t n){const uint8_t*b=p;uint8_t s=0;for(uint32_t i=0;i<n;i++)s=(uint8_t)(s+b[i]);return s;}
static const void *physmap(uint64_t p){if(!hhdm||p>UINT64_MAX-hhdm)return 0;return(const void*)(uintptr_t)(hhdm+p);}
static const psre_acpi_sdt_header_t *sdt(uint64_t p){const psre_acpi_sdt_header_t*x=physmap(p);if(!x||x->length<sizeof(*x)||x->length>1024*1024)return 0;return x;}
static void madt_parse(const psre_acpi_sdt_header_t*h){if(h->length<sizeof(madt_t))return;const uint8_t*p=(const uint8_t*)h+sizeof(madt_t),*end=(const uint8_t*)h+h->length;while(p+2<=end){const madt_entry_t*e=(const madt_entry_t*)p;if(e->length<2||p+e->length>end)break;if(e->type==0&&e->length>=8)++stats.local_apic_count;if(e->type==1&&e->length>=12)++stats.io_apic_count;p+=e->length;}}
static void count(const psre_acpi_sdt_header_t*h){++stats.table_count;if(sum8(h,h->length)!=0){stats.checksum_valid=false;return;}if(h->signature[0]=='A'&&h->signature[1]=='P'&&h->signature[2]=='I'&&h->signature[3]=='C'){++stats.madt_count;madt_parse(h);}else if(h->signature[0]=='M'&&h->signature[1]=='C'&&h->signature[2]=='F'&&h->signature[3]=='G')++stats.mcfg_count;else if(h->signature[0]=='F'&&h->signature[1]=='A'&&h->signature[2]=='C'&&h->signature[3]=='P')++stats.facp_count;else if(h->signature[0]=='H'&&h->signature[1]=='P'&&h->signature[2]=='E'&&h->signature[3]=='T')++stats.hpet_count;else ++stats.other_count;}
bool psre_acpi_initialize(void){for(size_t i=0;i<sizeof(stats);i++)((uint8_t*)&stats)[i]=0;stats.checksum_valid=true;hhdm=memory_map_hhdm_offset();if(!psre_rsdp_request.response||!psre_rsdp_request.response->address||!hhdm)return false;const rsdp_t*r=(const rsdp_t*)psre_rsdp_request.response->address;if(!r)return false;static const char sig[8]={'R','S','D',' ','P','T','R',' '};for(unsigned i=0;i<8;i++)if(r->sig[i]!=sig[i])return false;if(sum8(r,20)!=0)return false;if(r->revision>=2&&(r->length<sizeof(rsdp_t)||r->length>4096||sum8(r,r->length)!=0))return false;stats.rsdp_found=true;uint64_t rootpa=(r->revision>=2&&r->xsdt)?r->xsdt:r->rsdt;stats.xsdt_used=(r->revision>=2&&r->xsdt);const psre_acpi_sdt_header_t*root=sdt(rootpa);
 if(!root||sum8(root,root->length)!=0){stats.checksum_valid=false;return false;}
 const char *expected=stats.xsdt_used?"XSDT":"RSDT";
 for(unsigned k=0;k<4;k++)if(root->signature[k]!=expected[k]){stats.checksum_valid=false;return false;}
 uint32_t es=stats.xsdt_used?8:4;if(root->length<sizeof(*root))return false;uint32_t bytes=root->length-(uint32_t)sizeof(*root);if(bytes%es)return false;uint32_t n=bytes/es;if(n>4096)return false;const uint8_t*p=(const uint8_t*)root+sizeof(*root);for(uint32_t i=0;i<n;i++){uint64_t a=0;if(es==8){const uint64_t*v=(const uint64_t*)(const void*)(p+i*8);a=*v;}else{const uint32_t*v=(const uint32_t*)(const void*)(p+i*4);a=*v;}const psre_acpi_sdt_header_t*t=sdt(a);if(t)count(t);}return true;}
const psre_acpi_stats_t *psre_acpi_get_stats(void){return &stats;}
static void dec(uint32_t v){char b[11];unsigned i=10;b[i]=0;do{b[--i]=(char)('0'+v%10);v/=10;}while(v&&i);serial_write(&b[i]);}
void psre_acpi_report(void){serial_write("\n--- PSRE ACPI INVENTORY ---\nRSDP: ");serial_write(stats.rsdp_found?"found":"not found");serial_write(" root: ");serial_write(stats.xsdt_used?"XSDT":"RSDT");serial_write("\nSDTs: ");dec(stats.table_count);serial_write(" MADT: ");dec(stats.madt_count);serial_write(" LAPIC: ");dec(stats.local_apic_count);serial_write(" IOAPIC: ");dec(stats.io_apic_count);serial_write("\nMCFG: ");dec(stats.mcfg_count);serial_write(" FACP: ");dec(stats.facp_count);serial_write(" HPET: ");dec(stats.hpet_count);serial_write(" checksum-valid: ");serial_write(stats.checksum_valid?"yes":"no");serial_write("\n---------------------------\n");}

