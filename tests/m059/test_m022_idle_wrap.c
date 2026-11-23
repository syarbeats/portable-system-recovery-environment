/* Host regression: M022 interrupt endpoint must survive idle timeouts,
   accept Short Packet completions, and keep working past one ring lap. */
#include <psre/m022/interrupt.h>
#include <psre/m015/xhci_init.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/m019/usb_config.h>
#include <psre/m021/hid.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint64_t ticks; static psre_m015_status_t hc={.state=PSRE_M015_STATE_RUNNING,.max_slots=8};
static psre_m018_status_t dev={.address_assigned=true,.slot_id=1,.speed_id=3};
static uint8_t ctxmem[4096]; static psre_xhci_trb_t evq[64]; static int evn,evh;
static uint8_t *dma[16]; static int dmai; static int doorbells;
uint64_t psre_pit_ticks(void){return ++ticks;}
void serial_write(const char*s){(void)s;} void serial_write_char(char c){(void)c;}
const psre_m015_status_t *psre_m015_get_status(void){return &hc;}
const psre_m018_status_t *psre_m018_get_status(void){return &dev;}
const psre_m019_status_t *psre_m019_get_status(void){return 0;}
const psre_m021_status_t *psre_m021_get_status(void){return 0;}
bool psre_m018_get_context_memory(uint64_t*p,void**v,uint8_t*b){*p=0x1000;*v=ctxmem;*b=32;return true;}
bool psre_m015_dma_alloc_pages(uint32_t n,uint64_t*p,void**v){uint8_t*m=aligned_alloc(4096,4096u*n);memset(m,0,4096u*n);dma[dmai++]=m;*p=(uint64_t)(uintptr_t)m;*v=m;return true;}
bool psre_m015_submit_command(const psre_xhci_trb_t*c){(void)c;evq[evn++]=(psre_xhci_trb_t){0,1u<<24,(33u<<10)|((uint32_t)dev.slot_id<<24)};return true;}
bool psre_m015_ring_doorbell(uint8_t s,uint8_t t){(void)s;(void)t;++doorbells;return true;}
bool psre_m015_wait_event(psre_xhci_trb_t*e,uint32_t t){(void)t;if(evh<evn){*e=evq[evh++];return true;}return false;}
static void complete_ex(uint8_t slot,int blk,uint8_t dci,uint8_t cc,uint32_t residual,const uint8_t*data,int len){
    /* hardware writes report into endpoint data page (page 2 of the 3-page block) */
    memcpy(dma[blk]+8192,data,len);
    evq[evn++]=(psre_xhci_trb_t){0,(cc<<24)|residual,(32u<<10)|((uint32_t)slot<<24)|((uint32_t)dci<<16)};
}
static void complete(uint8_t dci,uint8_t cc,uint32_t residual,const uint8_t*data,int len){complete_ex(1,0,dci,cc,residual,data,len);}
#define CHECK(c) do{if(!(c)){printf("FAIL line %d: %s\n",__LINE__,#c);return 1;}}while(0)
int main(void){
    CHECK(psre_m022_configure_interrupt_in_multi(0x81,8,10));
    uint8_t buf[8],rep[8]={0,0,4,0,0,0,0,0};uint16_t n=0;
    /* 1. idle poll times out; this used to wedge the endpoint forever */
    CHECK(!psre_m022_interrupt_in_multi(0x81,buf,8,&n,1));
    CHECK(psre_m022_get_endpoint_status(0x81)->transfer_in_flight);
    /* 2. key arrives later as a Short Packet (cc=13, residual 3) */
    complete(3,13,3,rep,5);
    CHECK(psre_m022_interrupt_in_multi(0x81,buf,8,&n,1)); CHECK(n==5 && buf[2]==4);
    /* 3. well past one 255-TRB lap */
    for(int i=0;i<700;i++){
        evn=evh=0; rep[2]=(uint8_t)(i&0xff);
        complete(3,1,0,rep,8);
        CHECK(psre_m022_interrupt_in_multi(0x81,buf,8,&n,1)); CHECK(n==8 && buf[2]==(uint8_t)(i&0xff));
    }
    CHECK(psre_m022_get_endpoint_status(0x81)->transfer_count==701);
    /* 4. keyboard (slot 1) and mouse (slot 2) BOTH use endpoint 0x81 */
    dev.slot_id=2; CHECK(psre_m022_configure_interrupt_in_multi(0x81,4,10)); dev.slot_id=1;
    uint8_t mrep[4]={1,5,0,0}, krep[8]={0,0,9,0,0,0,0,0};
    evn=evh=0;
    CHECK(!psre_m022_interrupt_in_slot(2,0x81,buf,4,&n,1));   /* arm mouse */
    complete_ex(2,1,3,1,0,mrep,4); complete_ex(1,0,3,1,0,krep,8);
    CHECK(psre_m022_interrupt_in_slot(2,0x81,buf,4,&n,1)); CHECK(n==4 && buf[1]==5);
    CHECK(psre_m022_interrupt_in_slot(1,0x81,buf,8,&n,1)); CHECK(n==8 && buf[2]==9);
    puts("M022 idle/short-packet/ring-wrap host test: PASS"); return 0;
}
