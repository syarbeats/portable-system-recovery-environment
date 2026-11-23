#include <psre/m022/interrupt.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/m019/usb_config.h>
#include <psre/m021/hid.h>
#include <psre/m015/xhci_init.h>
#include <psre/m003/pit.h>
#include <psre/serial.h>
#include <stdint.h>
#include <stdbool.h>
#define M022_PAGE 4096u
#define M022_TRBS 256u
#define M022_LINK_TYPE 6u
#define M022_CONFIGURE_ENDPOINT 12u
#define M022_TRANSFER_EVENT 32u
#define M022_TRB_NORMAL 1u
#define M022_TRB_IOC (1u<<5)
#define M022_TRB_ISP (1u<<2)
#define M022_CYCLE 1u
#define M022_MAX_PACKET 1024u
#define M022_EP_CERR_VALUE 3u
#define M022_EP_TYPE_INTERRUPT_IN 7u
#define M022_SPINS 50000000u
typedef struct __attribute__((packed,aligned(16))) { uint64_t parameter; uint32_t status,control; } m022_trb_t;
typedef struct {
    psre_m022_status_t status;
    uint64_t dma_phys, ring_phys;
    uint8_t *dma_virt;
    uint8_t ring_cycle;
    uint16_t ring_enqueue;
    /* Completed report waiting to be consumed by the caller. */
    bool ready;
    uint16_t ready_len;
    uint8_t ready_buf[M022_MAX_PACKET];
} m022_endpoint_t;
static psre_m022_status_t aggregate;
static m022_endpoint_t endpoints[PSRE_M022_MAX_ENDPOINTS];
static uint8_t endpoint_count, context_bytes;
static void dec(uint32_t v){char b[10];unsigned n=0;if(!v){serial_write("0");return;}while(v&&n<sizeof(b)){b[n++]=(char)('0'+v%10u);v/=10u;}while(n)serial_write_char(b[--n]);}
static void hex8(uint8_t v){static const char h[]="0123456789ABCDEF";serial_write_char(h[v>>4]);serial_write_char(h[v&15u]);}
static m022_endpoint_t *find_endpoint_slot(uint8_t slot,uint8_t address){for(uint8_t i=0;i<endpoint_count;++i)if(endpoints[i].status.endpoint_address==address&&(!slot||endpoints[i].status.slot_id==slot))return &endpoints[i];return 0;}
static m022_endpoint_t *find_endpoint(uint8_t address){return find_endpoint_slot(0,address);}
static m022_endpoint_t *find_by_slot_dci(uint8_t slot,uint8_t dci){for(uint8_t i=0;i<endpoint_count;++i)if(endpoints[i].status.slot_id==slot&&endpoints[i].status.endpoint_id==dci)return &endpoints[i];return 0;}
/* Route one event-ring entry. Transfer Events are matched to their endpoint by
   (slot, DCI) so a report for the mouse is never lost while the keyboard is polled. */
static void dispatch_event(const psre_xhci_trb_t *ev){
    uint8_t type=(uint8_t)((ev->control>>10)&0x3fu);
    if(type!=M022_TRANSFER_EVENT){
        /* Command Completion (33) / Port Status Change (34) are benign here. */
        if(type!=33u&&type!=34u)++aggregate.unmatched_events;
        return;
    }
    uint8_t slot=(uint8_t)(ev->control>>24),dci=(uint8_t)((ev->control>>16)&0x1fu);
    m022_endpoint_t *e=find_by_slot_dci(slot,dci);
    if(!e){++aggregate.unmatched_events;return;}
    uint8_t cc=(uint8_t)(ev->status>>24);uint32_t residual=ev->status&0x00ffffffu;
    ++e->status.event_count;e->status.last_event_type=type;e->status.last_event_slot=slot;e->status.last_event_endpoint=dci;
    e->status.completion_code=cc;e->status.transfer_in_flight=false;e->status.transfer_pending=false;
    /* ISP is set on every TRB, so a report shorter than max-packet completes
       with Short Packet (13), which is a success for HID. */
    if(cc!=1u&&cc!=13u){++e->status.completion_errors;++aggregate.completion_errors;e->status.last_result=3u;return;}
    uint16_t requested=e->status.max_packet_size;
    if(residual>requested){e->status.last_result=5u;return;}
    uint16_t got=(uint16_t)(requested-residual);
    const uint8_t *db=e->dma_virt+2u*M022_PAGE;
    for(uint16_t n=0;n<got;++n)e->ready_buf[n]=db[n];
    e->ready_len=got;e->ready=true;
    e->status.last_requested=requested;e->status.last_actual=got;e->status.last_residual=(uint16_t)residual;e->status.last_result=1u;
    if(!got)++e->status.zero_length_transfers;
    ++e->status.transfer_count;++aggregate.transfer_count;
}
/* Drain the event ring. If wait_for is non-NULL, stop as soon as it has data or
   after timeout ticks. A timeout is the NORMAL idle case for an interrupt
   endpoint (the device NAKs until a key/mouse change), so the TRB stays armed. */
static void service_events(m022_endpoint_t *wait_for,uint32_t timeout){
    uint64_t start=psre_pit_ticks();
    for(uint64_t i=0;i<M022_SPINS;++i){
        psre_xhci_trb_t ev;
        if(psre_m015_wait_event(&ev,0u)){dispatch_event(&ev);if(wait_for&&wait_for->ready)return;continue;}
        if(!wait_for||psre_pit_ticks()-start>=timeout)return;
        __asm__ volatile("pause");
    }
}
static bool wait_command(uint8_t slot,uint32_t timeout){uint64_t start=psre_pit_ticks();for(uint64_t i=0;i<M022_SPINS;++i){psre_xhci_trb_t ev;if(psre_m015_wait_event(&ev,0u)){uint8_t type=(uint8_t)((ev.control>>10)&0x3fu),s=(uint8_t)(ev.control>>24);if(type==33u&&s==slot){aggregate.completion_code=(uint8_t)(ev.status>>24);return aggregate.completion_code==1u;}dispatch_event(&ev);}if(psre_pit_ticks()-start>=timeout)break;__asm__ volatile("pause");}++aggregate.timeout_count;return false;}
/* Arm one Normal TRB. At most one is outstanding per endpoint. */
static bool queue_transfer(m022_endpoint_t *e){
    if(e->status.transfer_in_flight)return true;
    m022_trb_t *ring=(m022_trb_t *)(e->dma_virt+M022_PAGE);
    m022_trb_t *t=&ring[e->ring_enqueue];
    t->parameter=e->dma_phys+2u*M022_PAGE;
    t->status=e->status.max_packet_size;
    t->control=(M022_TRB_NORMAL<<10)|M022_TRB_IOC|M022_TRB_ISP|e->ring_cycle;
    __asm__ volatile("mfence":::"memory");
    /* Advance; on reaching the Link TRB hand it to the xHC with the current
       cycle bit and toggle the producer cycle (previously the ring stopped
       working after 254 reports). */
    if(++e->ring_enqueue>=M022_TRBS-1u){
        ring[M022_TRBS-1u].control=(M022_LINK_TYPE<<10)|(1u<<1)|e->ring_cycle;
        e->ring_enqueue=0;e->ring_cycle^=1u;
        __asm__ volatile("mfence":::"memory");
    }
    ++e->status.transfer_attempts;++aggregate.transfer_attempts;
    e->status.transfer_pending=true;e->status.transfer_in_flight=true;
    if(!psre_m015_ring_doorbell(e->status.slot_id,e->status.endpoint_id)){
        e->status.transfer_pending=false;e->status.transfer_in_flight=false;
        e->status.last_result=4u;++e->status.doorbell_failures;++aggregate.doorbell_failures;
        return false;
    }
    return true;
}
bool psre_m022_configure_hid(void){const psre_m021_status_t *hid=psre_m021_get_status();const psre_m019_status_t *cfg=psre_m019_get_status();if(!hid||!hid->success||!cfg||!cfg->success){serial_write("[M022] Run m019-config and m021-probe first.\n");return false;}for(uint8_t i=0;i<hid->hid_interface_count;++i){const psre_m021_hid_interface_t *hi=&hid->interfaces[i];if(!hi->present||!hi->interrupt_in_endpoint)continue;for(uint8_t e=0;e<cfg->endpoint_count;++e){const psre_m019_endpoint_t *ep=&cfg->endpoints[e];if(ep->address==hi->interrupt_in_endpoint&&(ep->attributes&3u)==3u&&(ep->address&0x80u))return psre_m022_configure_interrupt_in(ep->address,ep->max_packet_size,ep->interval);}}serial_write("[M022] No validated HID Interrupt IN endpoint found.\n");return false;}
bool psre_m022_configure_interrupt_in(uint8_t a,uint16_t m,uint8_t i){return psre_m022_configure_interrupt_in_multi(a,m,i);}
bool psre_m022_configure_interrupt_in_multi(uint8_t address,uint16_t mps,uint8_t interval){
    const psre_m018_status_t *dev=psre_m018_get_status();
    /* Endpoints are keyed by (slot, address): a USB keyboard and mouse both use 0x81. */
    m022_endpoint_t *existing=find_endpoint_slot(dev?dev->slot_id:0,address);if(existing)return existing->status.endpoint_configured;
    if(endpoint_count>=PSRE_M022_MAX_ENDPOINTS)return false;
    const psre_m015_status_t *ctl=psre_m015_get_status();uint64_t dev_phys=0;void *dev_virt=0;
    if(!dev||!dev->address_assigned||!ctl||ctl->state!=PSRE_M015_STATE_RUNNING||!(address&0x80u)||!(address&0x0fu)||(address&0x70u)||!mps||mps>M022_MAX_PACKET||!interval)return false;
    m022_endpoint_t *e=&endpoints[endpoint_count];*e=(m022_endpoint_t){0};e->status.attempted=true;e->status.slot_id=dev->slot_id;e->status.endpoint_address=address;e->status.endpoint_id=(uint8_t)(2u*(address&0x0fu)+1u);e->status.max_packet_size=mps;e->status.interval=interval;
    if(!psre_m018_get_context_memory(&dev_phys,&dev_virt,&context_bytes)){e->status.failure_stage=2;return false;}
    if(!psre_m015_dma_alloc_pages(3u,&e->dma_phys,(void **)&e->dma_virt)){e->status.failure_stage=3;return false;}
    uint8_t *input=e->dma_virt;for(uint32_t n=0;n<M022_PAGE;++n)input[n]=0;uint32_t *ic=(uint32_t *)input;ic[1]=1u|(1u<<e->status.endpoint_id);
    for(uint32_t n=0;n<(uint32_t)context_bytes*2u;++n)input[context_bytes+n]=((uint8_t*)dev_virt)[n];
    /* Slot Context 'Context Entries' must cover the new DCI (required by real xHCI hardware). */
    uint32_t *slotctx=(uint32_t *)(input+context_bytes);
    if((slotctx[0]>>27)<e->status.endpoint_id)slotctx[0]=(slotctx[0]&0x07FFFFFFu)|((uint32_t)e->status.endpoint_id<<27);
    uint32_t *ep=(uint32_t *)(input+(uint32_t)context_bytes*(e->status.endpoint_id+1u));
    e->ring_phys=e->dma_phys+M022_PAGE;uint8_t *trmem=input+M022_PAGE;for(uint32_t n=0;n<M022_PAGE;++n)trmem[n]=0;
    uint32_t interval_field=0;if(dev->speed_id>=3u)interval_field=(uint32_t)(interval-1u);else{uint32_t period=(uint32_t)interval*8u,power=0,value=1u;while(value<period&&power<16u){value<<=1;++power;}interval_field=power?power-1u:0u;}if(interval_field>15u)interval_field=15u;
    ep[0]=interval_field<<16;
    /* xHCI Endpoint Context: CErr=3 (bits 2:1), Type=7 Interrupt IN (bits 5:3). */
    ep[1]=(M022_EP_CERR_VALUE<<1)|(M022_EP_TYPE_INTERRUPT_IN<<3)|((uint32_t)mps<<16);ep[2]=(uint32_t)(e->ring_phys|1u);ep[3]=(uint32_t)(e->ring_phys>>32);ep[4]=(uint32_t)mps|((uint32_t)mps<<16);
    m022_trb_t *ring=(m022_trb_t *)trmem;ring[M022_TRBS-1u].parameter=e->ring_phys;ring[M022_TRBS-1u].control=(M022_LINK_TYPE<<10)|(1u<<1)|M022_CYCLE;__asm__ volatile("mfence":::"memory");
    psre_xhci_trb_t cmd={0};cmd.parameter=e->dma_phys;cmd.control=(M022_CONFIGURE_ENDPOINT<<10)|((uint32_t)e->status.slot_id<<24)|M022_CYCLE;
    if(!psre_m015_submit_command(&cmd)||!wait_command(e->status.slot_id,500u)){e->status.failure_stage=4;return false;}
    e->ring_cycle=1u;e->ring_enqueue=0;e->status.endpoint_configured=true;++endpoint_count;
    serial_write("[M022/M026] Endpoint 0x");hex8(address);serial_write(" configured; ring retained.\n");return true;
}
bool psre_m022_interrupt_in_slot(uint8_t slot,uint8_t address,uint8_t *buffer,uint16_t capacity,uint16_t *actual,uint32_t timeout){
    m022_endpoint_t *e=find_endpoint_slot(slot,address);
    if(!e||!e->status.endpoint_configured||!buffer||!capacity||!actual||capacity>e->status.max_packet_size)return false;
    service_events(0,0u);                       /* route anything already completed */
    if(!e->ready){
        if(!queue_transfer(e))return false;
        service_events(e,timeout);
    }
    if(!e->ready){
        /* Idle: TRB remains armed with the xHC; the next poll will find the report. */
        ++e->status.timeout_count;++aggregate.timeout_count;e->status.last_result=2u;
        return false;
    }
    uint16_t got=e->ready_len<capacity?e->ready_len:capacity;
    for(uint16_t n=0;n<got;++n)buffer[n]=e->ready_buf[n];
    *actual=got;e->ready=false;e->ready_len=0;
    (void)queue_transfer(e);                    /* re-arm immediately so no report is missed */
    return true;
}
bool psre_m022_interrupt_in_multi(uint8_t address,uint8_t *buffer,uint16_t capacity,uint16_t *actual,uint32_t timeout){return psre_m022_interrupt_in_slot(0,address,buffer,capacity,actual,timeout);}
bool psre_m022_interrupt_in(uint8_t *b,uint16_t c,uint16_t *a,uint32_t t){if(!endpoint_count)return false;return psre_m022_interrupt_in_multi(endpoints[0].status.endpoint_address,b,c,a,t);}
const psre_m022_status_t *psre_m022_get_endpoint_status(uint8_t a){m022_endpoint_t *e=find_endpoint(a);return e?&e->status:0;}
const psre_m022_status_t *psre_m022_get_status(void){return &aggregate;}
void psre_m022_report(void){
    serial_write("\\n--- PSRE M022/M026 INTERRUPT TRANSFER ENGINE ---\\nEndpoints=");
    dec(endpoint_count);serial_write("/");dec(PSRE_M022_MAX_ENDPOINTS);
    serial_write(" Attempts=");dec(aggregate.transfer_attempts);
    serial_write(" Transfers=");dec(aggregate.transfer_count);
    serial_write(" Timeouts=");dec(aggregate.timeout_count);
    serial_write(" CC-errors=");dec(aggregate.completion_errors);
    serial_write(" Doorbell-errors=");dec(aggregate.doorbell_failures);
    serial_write(" Unmatched-events=");dec(aggregate.unmatched_events);
    serial_write("\\n");
    for(uint8_t i=0;i<endpoint_count;++i){
        psre_m022_status_t *v=&endpoints[i].status;
        serial_write(" EP=0x");hex8(v->endpoint_address);
        serial_write(" DCI=");dec(v->endpoint_id);
        serial_write(" attempts=");dec(v->transfer_attempts);
        serial_write(" success=");dec(v->transfer_count);
        serial_write(" timeout=");dec(v->timeout_count);
        serial_write(" cc-errors=");dec(v->completion_errors);
        serial_write(" unmatched=");dec(v->unmatched_events);
        serial_write(" in-flight=");serial_write(v->transfer_in_flight?"yes":"no");
        serial_write(" result=");dec(v->last_result);
        serial_write(" event=");dec(v->last_event_type);serial_write("/");
        dec(v->last_event_slot);serial_write("/");dec(v->last_event_endpoint);
        serial_write(" bytes=");dec(v->last_requested);serial_write("/");
        dec(v->last_actual);serial_write("/");dec(v->last_residual);
        serial_write("\\n");
    }
}
