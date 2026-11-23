#include <psre/m011/xhci.h>
#include <psre/m004/pci.h>
#include <psre/memory_map.h>
#include <psre/pmm.h>
#include <psre/serial.h>
#include <stdint.h>

#define XHCI_TRBS_PER_RING 256u
#define XHCI_LINK_TRB_TYPE 6u
#define XHCI_NOOP_CMD_TYPE 23u
#define XHCI_TRB_TYPE_SHIFT 10u
#define XHCI_TRB_CYCLE 1u
#define XHCI_MAX_DMA_ADDRESS 0xFFFFFFFFULL

static psre_m011_xhci_status_t status;
static uint64_t dma_phys;
static uint8_t *dma_virt;
static psre_xhci_trb_t *command_ring;
static psre_xhci_trb_t *event_ring;
static psre_xhci_erst_entry_t *erst;
static uint8_t command_cycle=1;

static void zero_bytes(void *ptr,uint64_t n) {
    uint8_t *p=(uint8_t*)ptr;
    for(uint64_t i=0;i<n;i++)p[i]=0;
}
static void hex(uint64_t v,unsigned n) {
    static const char h[]="0123456789ABCDEF";
    for(unsigned i=0;i<n;i++)serial_write_char(h[(v>>((n-1-i)*4))&15]);
}
static bool find_xhci(void) {
    const psre_pci_stats_t *ps=psre_pci_get_stats();
    const psre_pci_device_t *ds=psre_pci_get_devices();
    if(!ps || !ps->config_mechanism_1_available)return false;
    for(uint32_t i=0;i<ps->stored;i++) {
        const psre_pci_device_t *d=&ds[i];
        if(d->class_code!=0x0C || d->subclass!=0x03 || d->prog_if!=0x30)continue;
        status.controller_found=true;
        status.bus=d->bus;status.device=d->device;status.function=d->function;
        status.vendor_id=d->vendor_id;status.device_id=d->device_id;
        for(uint8_t b=0;b<d->bar_count;b++) {
            if(d->bars[b].kind==2 && d->bars[b].address) {
                status.bar_address=d->bars[b].address;
                break;
            }
        }
        return true;
    }
    return false;
}
static bool allocate_rings(void) {
    uint64_t hhdm=memory_map_hhdm_offset();
    if(!hhdm || dma_phys)return false;
    /* Three physically contiguous pages, below 4 GiB for broad xHCI DMA compatibility. */
    dma_phys=pmm_alloc_contiguous_pages(3,XHCI_MAX_DMA_ADDRESS);
    if(!dma_phys)return false;
    dma_virt=(uint8_t*)(uintptr_t)(hhdm+dma_phys);
    zero_bytes(dma_virt,3*PSRE_PAGE_SIZE);
    command_ring=(psre_xhci_trb_t*)dma_virt;
    event_ring=(psre_xhci_trb_t*)(dma_virt+PSRE_PAGE_SIZE);
    erst=(psre_xhci_erst_entry_t*)(dma_virt+2*PSRE_PAGE_SIZE);
    /* Command ring's final entry is a Link TRB back to its first TRB. */
    command_ring[XHCI_TRBS_PER_RING-1].parameter=dma_phys;
    command_ring[XHCI_TRBS_PER_RING-1].status=0;
    command_ring[XHCI_TRBS_PER_RING-1].control=
        (XHCI_LINK_TRB_TYPE<<XHCI_TRB_TYPE_SHIFT)|(1u<<1)|XHCI_TRB_CYCLE;
    /* Event Ring Segment Table: one 256-TRB segment. */
    erst[0].segment_base=dma_phys+PSRE_PAGE_SIZE;
    erst[0].segment_size=XHCI_TRBS_PER_RING;
    erst[0].reserved=0;
    status.dma_pages=3;
    status.dma_resources_ready=true;
    status.command_enqueue=0;
    status.event_dequeue=0;
    command_cycle=1;
    return true;
}
bool psre_m011_initialize(void) {
    status=(psre_m011_xhci_status_t){0};
    if(!find_xhci())return false;
    if(!allocate_rings())return false;
    return true;
}
const psre_m011_xhci_status_t *psre_m011_get_status(void){return &status;}
void psre_m011_report(void) {
    serial_write("\n--- PSRE M011 xHCI DRIVER FOUNDATION ---\n");
    if(!status.controller_found) {
        serial_write("[M011] No PCI xHCI controller found.\n");
        return;
    }
    serial_write("[xHCI] PCI ");hex(status.bus,2);serial_write(":");hex(status.device,2);
    serial_write(".");hex(status.function,1);serial_write(" VID:DID=");
    hex(status.vendor_id,4);serial_write(":");hex(status.device_id,4);
    serial_write("\n[xHCI] BAR MMIO=0x");hex(status.bar_address,16);serial_write("\n");
    serial_write("[M011] DMA ring resources: ");
    serial_write(status.dma_resources_ready?"READY\n":"NOT READY\n");
    if(status.dma_resources_ready) {
        serial_write("  DMA physical base=0x");hex(dma_phys,16);
        serial_write(" pages=");hex(status.dma_pages,2);
        serial_write("\n  Command ring entries=256, Event ring entries=256, ERST entries=1\n");
    }
    serial_write("[M011] Controller RUN/RESET and MMIO programming are NOT performed in this build.\n");
    serial_write("[M011] USB device enumeration is NOT active yet.\n");
}
bool psre_m011_software_selftest(void) {
    if(!status.dma_resources_ready || !command_ring || !event_ring || !erst)return false;
    zero_bytes(command_ring,PSRE_PAGE_SIZE);
    command_ring[XHCI_TRBS_PER_RING-1].parameter=dma_phys;
    command_ring[XHCI_TRBS_PER_RING-1].control=
        (XHCI_LINK_TRB_TYPE<<XHCI_TRB_TYPE_SHIFT)|(1u<<1)|XHCI_TRB_CYCLE;
    /* Queue a software-only No-Op Command TRB. It is not doorbelled to hardware. */
    command_ring[0].parameter=0;
    command_ring[0].status=0;
    command_ring[0].control=(XHCI_NOOP_CMD_TYPE<<XHCI_TRB_TYPE_SHIFT)|command_cycle;
    status.command_enqueue=1;
    bool ok=(command_ring[0].control>>XHCI_TRB_TYPE_SHIFT & 0x3Fu)==XHCI_NOOP_CMD_TYPE &&
            (command_ring[255].parameter==dma_phys) &&
            (erst[0].segment_base==dma_phys+PSRE_PAGE_SIZE) &&
            (erst[0].segment_size==XHCI_TRBS_PER_RING);
    serial_write(ok?"[M011] Software ring self-test PASS (no hardware doorbell).\n":
                     "[M011] Software ring self-test FAIL.\n");
    return ok;
}
void psre_m011_release(void) {
    if(dma_phys) (void)pmm_free_contiguous_pages(dma_phys,3);
    dma_phys=0;dma_virt=0;command_ring=0;event_ring=0;erst=0;
    status.dma_resources_ready=false;status.dma_pages=0;
    status.command_enqueue=0;status.event_dequeue=0;
    serial_write("[M011] DMA ring pages released.\n");
}
