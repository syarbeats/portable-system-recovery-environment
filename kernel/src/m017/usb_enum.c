#include <psre/m017/usb_enum.h>
#include <psre/m015/xhci_init.h>
#include <psre/m003/pit.h>
#include <psre/vmm.h>
#include <psre/serial.h>
#include <stdint.h>
#include <stdbool.h>

#define M017_MAX_MMIO_BYTES 0x10000u
#define M017_PORT_BASE 0x400u
#define M017_PORT_STRIDE 0x10u
#define M017_PORTSC_CCS (1u << 0)
#define M017_PORTSC_PED (1u << 1)
#define M017_PORTSC_SPEED (0xFu << 10)
#define M017_TRB_TYPE_SHIFT 10u
#define M017_TRB_ENABLE_SLOT 9u
#define M017_TRB_COMMAND_COMPLETION 33u
#define M017_TIMEOUT_TICKS 500u
#define M017_TIMEOUT_SPINS 50000000u

static psre_m017_status_t status;
static void dec(uint32_t v) {
    char b[10]; unsigned n=0; if(!v){serial_write("0");return;}
    while(v&&n<sizeof(b)){b[n++]=(char)('0'+v%10u);v/=10u;}
    while(n)serial_write_char(b[--n]);
}
bool psre_m017_enable_slot(uint8_t port) {
    status = (psre_m017_status_t){0};
    status.attempted = true; status.port = port;
    const psre_m015_status_t *m15 = psre_m015_get_status();
    if (!m15 || m15->state != PSRE_M015_STATE_RUNNING ||
        !m15->controller_running || !m15->busmaster_enabled) {
        serial_write("[M017] M015 controller must be RUNNING; run m015-init first.\n");
        status.timeout_stage = 1; return false;
    }
    if (!port || port > m15->max_ports) {
        serial_write("[M017] Invalid port number.\n"); status.timeout_stage=2; return false;
    }
    uint64_t bytes = (uint64_t)m15->cap_length + M017_PORT_BASE +
                     (uint64_t)m15->max_ports * M017_PORT_STRIDE;
    if (bytes > M017_MAX_MMIO_BYTES) {
        serial_write("[M017] Port MMIO window exceeds safety limit.\n");
        status.timeout_stage=3; return false;
    }
    uint32_t map_size=(uint32_t)((bytes+4095u)&~4095u);
    volatile uint32_t *bar=(volatile uint32_t *)vmm_map_mmio(m15->bar_address,map_size);
    if(!bar){serial_write("[M017] Cannot map xHCI BAR.\n");status.timeout_stage=4;return false;}
    uint32_t off=m15->cap_length+M017_PORT_BASE+((uint32_t)(port-1u)*M017_PORT_STRIDE);
    uint32_t portsc=bar[off/4u];
    bool unmap_ok=vmm_unmap_mmio((void *)bar,map_size);
    if(!unmap_ok){serial_write("[M017] WARNING: failed to unmap port-register window.\n");}
    if(!(portsc&M017_PORTSC_CCS)||!(portsc&M017_PORTSC_PED)) {
        serial_write("[M017] Port must be connected and enabled. Run m016-reset PORT first.\n");
        status.timeout_stage=5;return false;
    }
    uint8_t speed=(uint8_t)((portsc&M017_PORTSC_SPEED)>>10); status.speed_id=speed;
    serial_write("[M017] Submitting Enable Slot command for port ");dec(port);
    serial_write(" (speed ID ");dec(speed);serial_write(").\n");
    psre_xhci_trb_t cmd={0};
    cmd.control=(M017_TRB_ENABLE_SLOT<<M017_TRB_TYPE_SHIFT);
    if(!psre_m015_submit_command(&cmd)) {
        serial_write("[M017] Could not submit Enable Slot command.\n");
        status.timeout_stage=6;return false;
    }
    uint64_t start=psre_pit_ticks();
    for(uint64_t spins=0;spins<M017_TIMEOUT_SPINS;++spins) {
        psre_xhci_trb_t event;
        if(psre_m015_wait_command_event(&event,1u)) {
            uint8_t type=(uint8_t)((event.control>>10)&0x3Fu);
            if(type!=M017_TRB_COMMAND_COMPLETION) continue;
            uint8_t code=(uint8_t)(event.status>>24);
            uint8_t slot=(uint8_t)(event.control>>24);
            status.completion_code=code; status.slot_id=slot;
            if(code==1u && slot!=0u) {
                status.success=true;
                serial_write("[M017] Enable Slot completed successfully. Slot ID=");
                dec(slot);serial_write(".\n");
                serial_write("[M017] Slot allocated only; Address Device and descriptors are not yet implemented.\n");
                return true;
            }
            serial_write("[M017] Enable Slot failed. Completion code=");dec(code);
            serial_write(" Slot ID=");dec(slot);serial_write(".\n");
            status.timeout_stage=7;return false;
        }
        if(psre_pit_ticks()-start>=M017_TIMEOUT_TICKS)break;
        __asm__ volatile("pause");
    }
    status.timeout_stage=8;
    serial_write("[M017] Timed out waiting for Command Completion Event.\n");
    return false;
}
void psre_m017_report(void) {
    serial_write("\n--- PSRE M017 USB ENUMERATION STATUS ---\n");
    serial_write("[M017] Attempted=");serial_write(status.attempted?"yes":"no");
    serial_write(" Success=");serial_write(status.success?"yes":"no");
    serial_write(" Port=");dec(status.port);serial_write(" Slot ID=");dec(status.slot_id);
    serial_write(" Speed ID=");dec(status.speed_id); serial_write(" Completion code=");dec(status.completion_code);
    serial_write(" Stage=");dec(status.timeout_stage);serial_write("\n");
    serial_write("[M017] Current scope: Enable Slot + polled command completion only.\n");
}
const psre_m017_status_t *psre_m017_get_status(void){return &status;}
