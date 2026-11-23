#include <psre/m054/xhci_slots.h>
#include <psre/m015/xhci_init.h>
#include <psre/serial.h>
#include <stddef.h>
static psre_m054_slot_t slots[PSRE_M054_MAX_SLOTS+1u];
static psre_m054_status_t st;
static bool valid_state(psre_m054_slot_state_t s){return s>=PSRE_M054_SLOT_ENABLED && s<=PSRE_M054_SLOT_ERROR;}
bool psre_m054_initialize(void){
 if(st.initialized)return true;
 st=(psre_m054_status_t){0};
 const psre_m015_status_t *hc=psre_m015_get_status();
 if(!hc || hc->state!=PSRE_M015_STATE_RUNNING || !hc->max_slots){serial_write("[M054] xHCI is not running; slot registry initialized in offline mode.\n");st.capacity=PSRE_M054_MAX_SLOTS;st.initialized=true;return true;}
 st.capacity=hc->max_slots<PSRE_M054_MAX_SLOTS?hc->max_slots:PSRE_M054_MAX_SLOTS;
 st.initialized=true;serial_write("[M054] Multi-slot registry initialized; per-slot DMA contexts are allocated on registration.\n");return true;
}
bool psre_m054_register_slot(uint8_t id,uint8_t port,uint8_t speed){
 if(!st.initialized || !id || id>st.capacity || !port || !speed || slots[id].occupied){++st.rejected;return false;}
 const psre_m015_status_t *hc=psre_m015_get_status();
 if(!hc || hc->state!=PSRE_M015_STATE_RUNNING){++st.rejected;return false;}
 uint64_t dev_phys=0,ring_phys=0;void *dev_virt=0,*ring_virt=0;
 if(!psre_m015_dma_alloc_pages(1u,&dev_phys,&dev_virt) || !psre_m015_dma_alloc_pages(1u,&ring_phys,&ring_virt)){++st.rejected;return false;}
 if(!dev_virt || !ring_virt || (dev_phys&0x3Fu) || (ring_phys&0xFu)){++st.rejected;return false;}
 if(!psre_m015_set_dcbaa_entry(id,dev_phys)){++st.rejected;return false;}
 /* The device context and EP0 ring are zeroed; Address Device remains a separate stage. */
 uint8_t *d=(uint8_t *)dev_virt,*r=(uint8_t *)ring_virt;for(uint32_t i=0;i<4096u;i++){d[i]=0;r[i]=0;}
 slots[id]=(psre_m054_slot_t){.occupied=true,.slot_id=id,.root_port=port,.speed_id=speed,.state=PSRE_M054_SLOT_CONTEXT_READY,.device_context_phys=dev_phys,.device_context_virt=dev_virt,.ep0_ring_phys=ring_phys,.ep0_ring_virt=ring_virt,.generation=1u};
 ++st.active_slots;++st.allocations;++st.state_changes;return true;
}
bool psre_m054_set_identity(uint8_t id,uint16_t vid,uint16_t pid){if(!id||id>st.capacity||!slots[id].occupied)return false;slots[id].vendor_id=vid;slots[id].product_id=pid;return true;}
bool psre_m054_set_state(uint8_t id,psre_m054_slot_state_t state){if(!id||id>st.capacity||!slots[id].occupied||!valid_state(state))return false;slots[id].state=state;++st.state_changes;return true;}
bool psre_m054_unregister_slot(uint8_t id){
 if(!id||id>st.capacity||!slots[id].occupied)return false;
 /* DMA remains pinned: M015 does not expose a safe DMA free API while controller is active. */
 slots[id].state=PSRE_M054_SLOT_REMOVING;slots[id].occupied=false;if(st.active_slots)--st.active_slots;++st.state_changes;return true;
}
const psre_m054_slot_t *psre_m054_get_slot(uint8_t id){return id&&id<=st.capacity&&slots[id].occupied?&slots[id]:0;}
const psre_m054_status_t *psre_m054_get_status(void){return &st;}
static void dec(uint32_t v){char b[10];unsigned n=0;if(!v){serial_write("0");return;}while(v&&n<sizeof b){b[n++]=(char)('0'+v%10u);v/=10u;}while(n)serial_write_char(b[--n]);}
void psre_m054_report(void){serial_write("[M054] initialized=");serial_write(st.initialized?"yes":"no");serial_write(" capacity=");dec(st.capacity);serial_write(" active_slots=");dec(st.active_slots);serial_write(" allocations=");dec(st.allocations);serial_write(" rejected=");dec(st.rejected);serial_write(" state_changes=");dec(st.state_changes);serial_write("\n");}
