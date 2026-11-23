#include <psre/m045/bulk.h>
#include <psre/m015/xhci_init.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/m003/pit.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
static uint8_t devctx[4096]; static uint8_t pages[3*4096] __attribute__((aligned(4096))); static uint32_t event_calls; static uint64_t ticks; static uint8_t last_ep;
const psre_m018_status_t *psre_m018_get_status(void){static psre_m018_status_t s={.address_assigned=true,.slot_id=1,.speed_id=3};return &s;}
bool psre_m018_get_context_memory(uint64_t *p,void **v,uint8_t *c){*p=0x200000;*v=devctx;*c=32;return true;}
const psre_m015_status_t *psre_m015_get_status(void){static psre_m015_status_t s={.state=PSRE_M015_STATE_RUNNING};return &s;}
bool psre_m015_dma_alloc_pages(uint32_t n,uint64_t *p,void **v){(void)n;*p=0x300000;*v=pages;return true;}
bool psre_m015_submit_command(const psre_xhci_trb_t *c){(void)c;return true;}
bool psre_m015_wait_event(psre_xhci_trb_t *e,uint32_t t){(void)t;memset(e,0,sizeof(*e));e->status=1u<<24;if(event_calls++%2u==0)e->control=(33u<<10)|(1u<<24);else e->control=(32u<<10)|((uint32_t)last_ep<<16)|(1u<<24);return true;}
bool psre_m015_ring_doorbell(uint8_t s,uint8_t t){(void)s;last_ep=t;return true;}
uint64_t psre_pit_ticks(void){return ticks++;}
void serial_write(const char *s){(void)s;} void serial_write_char(char c){(void)c;}
int main(void){uint8_t out[32],in[32];uint32_t actual=0;memset(out,0x5a,sizeof(out));assert(psre_m045_configure_bulk_endpoint(0x02,512));assert(psre_m045_configure_bulk_endpoint(0x81,512));assert(!psre_m045_configure_bulk_endpoint(0x01,0));assert(psre_m045_bulk_transfer(0x02,out,sizeof(out),10,&actual));assert(actual==32);assert(psre_m045_bulk_transfer(0x81,in,sizeof(in),10,&actual));assert(actual==32);assert(psre_m045_get_status()->configured_endpoints==2);assert(psre_m045_get_status()->transfers==2);assert(psre_m045_get_endpoint_status(0x81)->in);return 0;}
