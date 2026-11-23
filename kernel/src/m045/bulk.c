#include <psre/m045/bulk.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/m015/xhci_init.h>
#include <psre/m003/pit.h>
#include <psre/serial.h>
#include <stdint.h>
#include <stdbool.h>
#define PAGE 4096u
#define TRBS 256u
#define LINK_TYPE 6u
#define CONFIGURE_ENDPOINT 12u
#define TRANSFER_EVENT 32u
#define TRB_NORMAL 1u
#define IOC (1u<<5)
#define ISP (1u<<2)
#define SPINS 50000000u
typedef struct __attribute__((packed,aligned(16))) { uint64_t parameter; uint32_t status,control; } trb_t;
typedef struct { psre_m045_endpoint_status_t status; uint64_t dma_phys,ring_phys; uint8_t *dma_virt; uint16_t enqueue; uint8_t cycle; } endpoint_t;
static endpoint_t eps[PSRE_M045_MAX_ENDPOINTS]; static uint8_t count,ctx_size; static psre_m045_status_t aggregate;
static endpoint_t *find(uint8_t a){for(uint8_t i=0;i<count;++i)if(eps[i].status.address==a)return &eps[i];return 0;}
static bool wait_command(uint8_t slot,uint32_t timeout){uint64_t start=psre_pit_ticks();for(uint64_t i=0;i<SPINS;++i){psre_xhci_trb_t ev;if(psre_m015_wait_event(&ev,1u)){uint8_t type=(uint8_t)((ev.control>>10)&0x3fu),s=(uint8_t)(ev.control>>24);if(type==33u&&s==slot)return ((ev.status>>24)&0xffu)==1u;}if(psre_pit_ticks()-start>=timeout)break;__asm__ volatile("pause");}++aggregate.timeouts;return false;}
static bool wait_transfer(endpoint_t *e,uint32_t timeout,uint32_t *residual){uint64_t start=psre_pit_ticks();for(uint64_t i=0;i<SPINS;++i){psre_xhci_trb_t ev;if(psre_m015_wait_event(&ev,1u)){uint8_t type=(uint8_t)((ev.control>>10)&0x3fu),s=(uint8_t)(ev.control>>24),ep=(uint8_t)((ev.control>>16)&0x1fu);if(type==TRANSFER_EVENT&&s==e->status.slot_id&&ep==e->status.endpoint_id){e->status.completion_code=(uint8_t)(ev.status>>24);*residual=ev.status&0x00ffffffu;return e->status.completion_code==1u;}}if(psre_pit_ticks()-start>=timeout)break;__asm__ volatile("pause");}++e->status.timeouts;++aggregate.timeouts;return false;}
bool psre_m045_configure_bulk_endpoint(uint8_t address,uint16_t mps){endpoint_t *old=find(address);if(old)return old->status.configured;if(count>=PSRE_M045_MAX_ENDPOINTS||!(address&0x0fu)||(address&0x70u)||!mps||mps>1024u)return false;const psre_m018_status_t *dev=psre_m018_get_status();const psre_m015_status_t *ctl=psre_m015_get_status();uint64_t dev_phys=0;void *dev_virt=0;if(!dev||!dev->address_assigned||!ctl||ctl->state!=PSRE_M015_STATE_RUNNING||!psre_m018_get_context_memory(&dev_phys,&dev_virt,&ctx_size))return false;
 endpoint_t *e=&eps[count];*e=(endpoint_t){0};e->status.address=address;e->status.in=(address&0x80u)!=0;e->status.slot_id=dev->slot_id;e->status.endpoint_id=(uint8_t)(2u*(address&0x0fu)+(e->status.in?1u:0u));e->status.max_packet=mps;
 if(!psre_m015_dma_alloc_pages(3u,&e->dma_phys,(void **)&e->dma_virt)) return false;
 for(uint32_t i=0;i<3u*PAGE;++i) e->dma_virt[i]=0;
 uint32_t *ic=(uint32_t *)e->dma_virt;ic[1]=1u|(1u<<e->status.endpoint_id);for(uint32_t i=0;i<(uint32_t)ctx_size*2u;++i)e->dma_virt[ctx_size+i]=((uint8_t*)dev_virt)[i];
 uint32_t *ep=(uint32_t *)(e->dma_virt+(uint32_t)ctx_size*(e->status.endpoint_id+1u));e->ring_phys=e->dma_phys+PAGE;uint8_t *ring_mem=e->dma_virt+PAGE;trb_t *ring=(trb_t *)ring_mem;ring[TRBS-1u].parameter=e->ring_phys;ring[TRBS-1u].control=(LINK_TYPE<<10)|(1u<<1)|1u;
 /* xHCI endpoint type: Bulk OUT=2, Bulk IN=6. */ep[0]=0u;ep[1]=((e->status.in?6u:2u)<<3)|((uint32_t)mps<<16);ep[2]=(uint32_t)(e->ring_phys|1u);ep[3]=(uint32_t)(e->ring_phys>>32);ep[4]=mps;__asm__ volatile("mfence":::"memory");psre_xhci_trb_t cmd={0};cmd.parameter=e->dma_phys;cmd.control=(CONFIGURE_ENDPOINT<<10)|((uint32_t)e->status.slot_id<<24)|1u;if(!psre_m015_submit_command(&cmd)||!wait_command(e->status.slot_id,500u)){++aggregate.errors;return false;}e->cycle=1u;e->status.configured=true;++count;++aggregate.configured_endpoints;return true;}
bool psre_m045_bulk_transfer(uint8_t address,void *buffer,uint32_t length,uint32_t timeout,uint32_t *actual){endpoint_t *e=find(address);if(!e||!e->status.configured||!buffer||!length||length>PSRE_M045_MAX_TRANSFER||!timeout||!actual||e->status.pending||e->enqueue>=TRBS-1u)return false;uint8_t *dma=e->dma_virt+2u*PAGE;if(!e->status.in){const uint8_t *src=(const uint8_t*)buffer;for(uint32_t i=0;i<length;++i)dma[i]=src[i];}trb_t *ring=(trb_t *)(e->dma_virt+PAGE);trb_t *t=&ring[e->enqueue];t->parameter=e->dma_phys+2u*PAGE;t->status=length;t->control=(TRB_NORMAL<<10)|IOC|(e->status.in?ISP:0u)|e->cycle;__asm__ volatile("mfence":::"memory");e->status.pending=true;if(!psre_m015_ring_doorbell(e->status.slot_id,e->status.endpoint_id)){e->status.pending=false;++e->status.errors;++aggregate.errors;return false;}uint32_t residual=length;bool ok=wait_transfer(e,timeout,&residual);e->status.pending=false;if(!ok||residual>length){++e->status.errors;++aggregate.errors;return false;}uint32_t done=length-residual;if(e->status.in){uint8_t *dst=(uint8_t*)buffer;for(uint32_t i=0;i<done;++i)dst[i]=dma[i];}*actual=done;aggregate.last_residual=residual;++e->status.transfers;++aggregate.transfers;++e->enqueue;return true;}
const psre_m045_endpoint_status_t *psre_m045_get_endpoint_status(uint8_t a){endpoint_t *e=find(a);return e?&e->status:0;}const psre_m045_status_t *psre_m045_get_status(void){return &aggregate;}
void psre_m045_report(void){serial_write("\n--- PSRE M045 xHCI BULK TRANSFER ---\nEndpoints=");char b[12];uint32_t v=count,n=0;if(!v)serial_write("0");else{while(v&&n<sizeof(b)){b[n++]=(char)('0'+v%10u);v/=10u;}while(n)serial_write_char(b[--n]);}serial_write(" Transfers=");v=aggregate.transfers;n=0;if(!v)serial_write("0");else{while(v&&n<sizeof(b)){b[n++]=(char)('0'+v%10u);v/=10u;}while(n)serial_write_char(b[--n]);}serial_write(" Timeouts/errors tracked.\n");}
