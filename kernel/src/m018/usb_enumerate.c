#include <psre/m018/usb_enumerate.h>
#include <psre/m017/usb_enum.h>
#include <psre/m015/xhci_init.h>
#include <psre/m011/xhci.h>
#include <psre/m003/pit.h>
#include <psre/serial.h>
#include <stdint.h>
#include <stdbool.h>

#define M018_TRB_TYPE_SHIFT 10u
#define M018_TRB_CYCLE       (1u << 0)
#define M018_TRB_CHAIN       (1u << 4)
#define M018_TRB_IOC         (1u << 5)
#define M018_TRB_IDT         (1u << 6)
#define M018_TRB_DIR_IN      (1u << 16)
#define M018_TRB_SETUP       2u
#define M018_TRB_DATA        3u
#define M018_TRB_STATUS      4u
#define M018_TRB_ADDRESS_DEVICE 11u
#define M018_TRB_COMMAND_COMPLETION 33u
#define M018_TRB_TRANSFER_EVENT 32u
#define M018_TIMEOUT_TICKS 500u
#define M018_EVENT_SPINS 50000000u
#define M018_CONTEXT_PAGES 4u
#define M018_PAGE_SIZE 4096u
#define M018_EP0_ID 1u
#define M018_DESC_DEVICE 1u
#define M018_DESC_LENGTH 18u

typedef struct __attribute__((packed, aligned(16))) {
    uint64_t parameter;
    uint32_t status;
    uint32_t control;
} m018_trb_t;

static psre_m018_status_t status;
static uint64_t dma_phys;
static uint8_t *dma_virt;
static uint8_t context_bytes;
static uint8_t transfer_cycle = 1;
static uint16_t transfer_enqueue = 3;

static void dec(uint32_t v) {
    char b[10]; unsigned n=0; if(!v){serial_write("0");return;}
    while(v && n<sizeof(b)){b[n++]=(char)('0'+v%10u);v/=10u;}
    while(n)serial_write_char(b[--n]);
}
static void hex16(uint16_t v) {
    static const char h[]="0123456789ABCDEF";
    for(int i=3;i>=0;--i) serial_write_char(h[(v>>(i*4))&15u]);
}
static bool wait_for_event(uint8_t expected_type, uint8_t slot,
                           uint8_t endpoint, psre_xhci_trb_t *matched) {
    uint64_t start=psre_pit_ticks();
    for(uint64_t spins=0;spins<M018_EVENT_SPINS;++spins) {
        psre_xhci_trb_t ev;
        if(psre_m015_wait_event(&ev,1u)) {
            uint8_t type=(uint8_t)((ev.control>>10)&0x3Fu);
            if(type!=expected_type) continue;
            uint8_t ev_slot=(uint8_t)(ev.control>>24);
            uint8_t ev_ep=(uint8_t)((ev.control>>16)&0x1Fu);
            if(ev_slot!=slot || (expected_type==M018_TRB_TRANSFER_EVENT && ev_ep!=endpoint))
                continue;
            if(matched)*matched=ev;
            status.completion_code=(uint8_t)(ev.status>>24);
            return true;
        }
        if(psre_pit_ticks()-start>=M018_TIMEOUT_TICKS) break;
        __asm__ volatile("pause");
    }
    return false;
}
static bool submit_address_device(void) {
    uint8_t *input=dma_virt;
    uint8_t *device=dma_virt+M018_PAGE_SIZE;
    uint64_t input_phys=dma_phys;
    uint64_t device_phys=dma_phys+M018_PAGE_SIZE;
    uint64_t ep_ring_phys=dma_phys+2u*M018_PAGE_SIZE;
    uint32_t *ic=(uint32_t *)input;
    uint32_t *slot=(uint32_t *)(input+context_bytes);
    uint32_t *ep0=(uint32_t *)(input+2u*context_bytes);
    uint16_t mps=(status.speed_id>=3u)?64u:8u;
    if(status.speed_id>=4u)mps=512u;
    status.max_packet_size0=mps;
    ic[1]=3u; /* Add Slot Context + Endpoint 0 Context. */
    slot[0]=((uint32_t)status.speed_id<<20)|(1u<<27); /* speed, one context */
    slot[1]=(uint32_t)status.port<<16; /* root hub port number */
    ep0[1]=(3u<<1)|(4u<<3)|((uint32_t)mps<<16); /* CErr=3, control EP */
    ep0[2]=(uint32_t)(ep_ring_phys|1u);
    ep0[3]=(uint32_t)(ep_ring_phys>>32);
    ep0[4]=8u; /* average TRB length */
    (void)device;
    if(!psre_m015_set_dcbaa_entry(status.slot_id,device_phys)) return false;
    psre_xhci_trb_t cmd={0};
    cmd.parameter=input_phys;
    cmd.control=(M018_TRB_ADDRESS_DEVICE<<M018_TRB_TYPE_SHIFT)|((uint32_t)status.slot_id<<24)|M018_TRB_CYCLE;
    if(!psre_m015_submit_command(&cmd)) return false;
    psre_xhci_trb_t ev;
    if(!wait_for_event(M018_TRB_COMMAND_COMPLETION,status.slot_id,0,&ev)) return false;
    if(status.completion_code!=1u) return false;
    status.address_assigned=true;
    return true;
}
static bool read_device_descriptor(void) {
    m018_trb_t *ring=(m018_trb_t *)(dma_virt+2u*M018_PAGE_SIZE);
    uint8_t *buffer=dma_virt+3u*M018_PAGE_SIZE;
    uint64_t buffer_phys=dma_phys+3u*M018_PAGE_SIZE;
    for(uint32_t i=0;i<256u;++i)ring[i]=(m018_trb_t){0};
    ring[255].control=(6u<<M018_TRB_TYPE_SHIFT)|(1u<<1)|transfer_cycle;
    /* GET_DESCRIPTOR(Device), 18 bytes, bmRequestType=0x80, bRequest=6. */
    uint64_t setup=(uint64_t)0x80u|((uint64_t)6u<<8)|((uint64_t)0x0100u<<16)|((uint64_t)18u<<48);
    ring[0].parameter=setup;
    ring[0].status=8u;
    ring[0].control=(M018_TRB_SETUP<<M018_TRB_TYPE_SHIFT)|M018_TRB_IDT|M018_TRB_CHAIN|(3u<<16)|transfer_cycle;
    ring[1].parameter=buffer_phys;
    ring[1].status=M018_DESC_LENGTH;
    ring[1].control=(M018_TRB_DATA<<M018_TRB_TYPE_SHIFT)|M018_TRB_DIR_IN|M018_TRB_CHAIN|transfer_cycle;
    ring[2].control=(M018_TRB_STATUS<<M018_TRB_TYPE_SHIFT)|M018_TRB_IOC|transfer_cycle;
    __asm__ volatile("mfence" ::: "memory");
    if(!psre_m015_ring_doorbell(status.slot_id,M018_EP0_ID)) return false;
    psre_xhci_trb_t ev;
    if(!wait_for_event(M018_TRB_TRANSFER_EVENT,status.slot_id,M018_EP0_ID,&ev)) return false;
    if(status.completion_code!=1u) return false;
    if(buffer[0]<M018_DESC_LENGTH || buffer[1]!=M018_DESC_DEVICE) return false;
    status.usb_bcd=(uint16_t)(buffer[2]|((uint16_t)buffer[3]<<8));
    status.device_class=buffer[4];
    status.device_subclass=buffer[5];
    status.device_protocol=buffer[6];
    status.max_packet_size0=buffer[7];
    status.manufacturer_index=buffer[14];
    status.product_index=buffer[15];
    status.serial_index=buffer[16];
    status.configuration_count=buffer[17];
    status.vendor_id=(uint16_t)(buffer[8]|((uint16_t)buffer[9]<<8));
    status.product_id=(uint16_t)(buffer[10]|((uint16_t)buffer[11]<<8));
    status.descriptor_read=true;
    return true;
}
bool psre_m018_enumerate(uint8_t port) {
    /* Multi-device: each enumeration starts from clean per-device state. Earlier
       devices keep their slot, contexts and interrupt rings (owned by M022). */
    status=(psre_m018_status_t){0}; transfer_cycle=1; transfer_enqueue=3; dma_phys=0; dma_virt=0;
    status.attempted=true; status.port=port;
    const psre_m015_status_t *m15=psre_m015_get_status();
    const psre_m017_status_t *m17=psre_m017_get_status();
    if(!m15 || m15->state!=PSRE_M015_STATE_RUNNING || !m17 ||
       !m17->success || m17->port!=port || !m17->slot_id) {
        serial_write("[M018] Run M015 init and successful m017-enable-slot for this port first.\n");
        status.failure_stage=1; return false;
    }
    status.slot_id=m17->slot_id; status.speed_id=m17->speed_id;
    context_bytes=psre_m015_context_size();
    if(context_bytes!=32u && context_bytes!=64u) {
        serial_write("[M018] Invalid xHCI context size.\n");status.failure_stage=2;return false;
    }
    if(!psre_m015_dma_alloc_pages(M018_CONTEXT_PAGES,&dma_phys,(void **)&dma_virt)) {
        serial_write("[M018] DMA allocation failed.\n");status.failure_stage=3;return false;
    }
    serial_write("[M018] Allocated Input Context, Device Context, EP0 ring and descriptor buffer.\n");
    if(!submit_address_device()) {
        serial_write("[M018] Address Device command failed or timed out; DMA remains pinned.\n");
        status.failure_stage=4;return false;
    }
    serial_write("[M018] USB address assigned by xHCI. Reading Device Descriptor...\n");
    if(!read_device_descriptor()) {
        serial_write("[M018] Device Descriptor control transfer failed or timed out.\n");
        status.failure_stage=5;return false;
    }
    serial_write("[M018] Device Descriptor received: VID=0x");hex16(status.vendor_id);
    serial_write(" PID=0x");hex16(status.product_id);serial_write(" USB=0x");hex16(status.usb_bcd);
    serial_write(" Class=");dec(status.device_class);serial_write(" Subclass=");dec(status.device_subclass);
    serial_write(" Protocol=");dec(status.device_protocol);serial_write(" EP0 MPS=");dec(status.max_packet_size0);
    serial_write("\n[M018] Device address and Device Descriptor stage completed.\n");
    return true;
}
void psre_m018_report(void) {
    serial_write("\n--- PSRE M018 USB ENUMERATION STATUS ---\n");
    serial_write("Attempted=");serial_write(status.attempted?"yes":"no");
    serial_write(" Addressed=");serial_write(status.address_assigned?"yes":"no");
    serial_write(" Descriptor=");serial_write(status.descriptor_read?"read":"not-read");
    serial_write(" Port=");dec(status.port);serial_write(" Slot=");dec(status.slot_id);
    serial_write(" Speed ID=");dec(status.speed_id);serial_write(" VID=0x");hex16(status.vendor_id);
    serial_write(" PID=0x");hex16(status.product_id);serial_write(" Failure stage=");dec(status.failure_stage);
    serial_write("\n[M018] Configuration descriptors, strings, hubs and Mass Storage are not implemented.\n");
}
const psre_m018_status_t *psre_m018_get_status(void){return &status;}


bool psre_m018_control_in(uint8_t request_type, uint8_t request,
                          uint16_t value, uint16_t index,
                          uint16_t length, uint8_t *output,
                          uint16_t output_capacity) {
    if (!status.descriptor_read || !dma_virt || !output || !length ||
        length > 1024u || length > output_capacity ||
        transfer_enqueue > 252u)
        return false;
    m018_trb_t *ring=(m018_trb_t *)(dma_virt+2u*M018_PAGE_SIZE);
    uint8_t *buffer=dma_virt+3u*M018_PAGE_SIZE;
    uint64_t buffer_phys=dma_phys+3u*M018_PAGE_SIZE;
    for(uint32_t i=0;i<3u;++i) ring[transfer_enqueue+i]=(m018_trb_t){0};
    uint64_t setup=(uint64_t)request_type |
        ((uint64_t)request<<8) | ((uint64_t)value<<16) |
        ((uint64_t)index<<32) | ((uint64_t)length<<48);
    ring[transfer_enqueue].parameter=setup;
    ring[transfer_enqueue].status=8u;
    ring[transfer_enqueue].control=(M018_TRB_SETUP<<M018_TRB_TYPE_SHIFT)|
        M018_TRB_IDT|M018_TRB_CHAIN|(3u<<16)|transfer_cycle;
    ring[transfer_enqueue+1u].parameter=buffer_phys;
    ring[transfer_enqueue+1u].status=length;
    ring[transfer_enqueue+1u].control=(M018_TRB_DATA<<M018_TRB_TYPE_SHIFT)|
        M018_TRB_DIR_IN|M018_TRB_CHAIN|transfer_cycle;
    ring[transfer_enqueue+2u].control=(M018_TRB_STATUS<<M018_TRB_TYPE_SHIFT)|
        M018_TRB_IOC|transfer_cycle;
    __asm__ volatile("mfence" ::: "memory");
    if(!psre_m015_ring_doorbell(status.slot_id,M018_EP0_ID)) return false;
    psre_xhci_trb_t ev;
    if(!wait_for_event(M018_TRB_TRANSFER_EVENT,status.slot_id,M018_EP0_ID,&ev) ||
       status.completion_code!=1u)
        return false;
    for(uint16_t i=0;i<length;++i) output[i]=buffer[i];
    transfer_enqueue=(uint16_t)(transfer_enqueue+3u);
    return true;
}


bool psre_m018_control_no_data(uint8_t request_type, uint8_t request,
                               uint16_t value, uint16_t index) {
    if (!status.descriptor_read || !dma_virt || transfer_enqueue > 253u)
        return false;
    m018_trb_t *ring=(m018_trb_t *)(dma_virt+2u*M018_PAGE_SIZE);
    for(uint32_t i=0;i<2u;++i) ring[transfer_enqueue+i]=(m018_trb_t){0};
    uint64_t setup=(uint64_t)request_type |
        ((uint64_t)request<<8) | ((uint64_t)value<<16) |
        ((uint64_t)index<<32);
    ring[transfer_enqueue].parameter=setup;
    ring[transfer_enqueue].status=8u;
    /* TRT=No Data. Status stage for an OUT request is IN. */
    ring[transfer_enqueue].control=(M018_TRB_SETUP<<M018_TRB_TYPE_SHIFT)|
        M018_TRB_IDT|transfer_cycle;
    ring[transfer_enqueue+1u].control=(M018_TRB_STATUS<<M018_TRB_TYPE_SHIFT)|
        M018_TRB_DIR_IN|M018_TRB_IOC|transfer_cycle;
    __asm__ volatile("mfence" ::: "memory");
    if(!psre_m015_ring_doorbell(status.slot_id,M018_EP0_ID)) return false;
    psre_xhci_trb_t ev;
    if(!wait_for_event(M018_TRB_TRANSFER_EVENT,status.slot_id,M018_EP0_ID,&ev) ||
       status.completion_code!=1u)
        return false;
    transfer_enqueue=(uint16_t)(transfer_enqueue+2u);
    return true;
}


bool psre_m018_control_out(uint8_t request_type, uint8_t request,
                           uint16_t value, uint16_t index,
                           uint16_t length, const uint8_t *input,
                           uint16_t input_capacity) {
    if (!status.descriptor_read || !dma_virt || !input || !length ||
        length > 1024u || length > input_capacity || transfer_enqueue > 252u)
        return false;
    m018_trb_t *ring=(m018_trb_t *)(dma_virt+2u*M018_PAGE_SIZE);
    uint8_t *buffer=dma_virt+3u*M018_PAGE_SIZE;
    uint64_t buffer_phys=dma_phys+3u*M018_PAGE_SIZE;
    for (uint16_t i=0;i<length;++i) buffer[i]=input[i];
    for (uint32_t i=0;i<3u;++i) ring[transfer_enqueue+i]=(m018_trb_t){0};
    uint64_t setup=(uint64_t)request_type |
        ((uint64_t)request<<8) | ((uint64_t)value<<16) |
        ((uint64_t)index<<32) | ((uint64_t)length<<48);
    ring[transfer_enqueue].parameter=setup;
    ring[transfer_enqueue].status=8u;
    /* TRT=OUT data stage. */
    ring[transfer_enqueue].control=(M018_TRB_SETUP<<M018_TRB_TYPE_SHIFT)|
        M018_TRB_IDT|M018_TRB_CHAIN|(2u<<16)|transfer_cycle;
    ring[transfer_enqueue+1u].parameter=buffer_phys;
    ring[transfer_enqueue+1u].status=length;
    ring[transfer_enqueue+1u].control=(M018_TRB_DATA<<M018_TRB_TYPE_SHIFT)|
        M018_TRB_CHAIN|transfer_cycle;
    /* Status stage for an OUT data request is IN. */
    ring[transfer_enqueue+2u].control=(M018_TRB_STATUS<<M018_TRB_TYPE_SHIFT)|
        M018_TRB_DIR_IN|M018_TRB_IOC|transfer_cycle;
    __asm__ volatile("mfence" ::: "memory");
    if (!psre_m015_ring_doorbell(status.slot_id,M018_EP0_ID)) return false;
    psre_xhci_trb_t ev;
    if (!wait_for_event(M018_TRB_TRANSFER_EVENT,status.slot_id,M018_EP0_ID,&ev) ||
        status.completion_code!=1u) return false;
    transfer_enqueue=(uint16_t)(transfer_enqueue+3u);
    return true;
}

bool psre_m018_get_context_memory(uint64_t *device_context_phys,
                                  void **device_context_virt,
                                  uint8_t *context_size) {
    if (!status.address_assigned || !dma_virt || !device_context_phys ||
        !device_context_virt || !context_size ||
        (context_bytes != 32u && context_bytes != 64u))
        return false;
    *device_context_phys = dma_phys + M018_PAGE_SIZE;
    *device_context_virt = dma_virt + M018_PAGE_SIZE;
    *context_size = context_bytes;
    return true;
}
