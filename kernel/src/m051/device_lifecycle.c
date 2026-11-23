#include <psre/m051/device_lifecycle.h>
#include <psre/m015/xhci_init.h>
#include <psre/m016/usb_ports.h>
#include <psre/m003/pit.h>
#include <psre/serial.h>
#define PORTSC_CCS (1u<<0)
#define PORTSC_CSC (1u<<17)
typedef struct { psre_m051_device_state_t state; bool known, connected; uint32_t last_portsc; } port_state_t;
static port_state_t ports[PSRE_M051_MAX_PORTS+1u];
static psre_m051_event_t queue[PSRE_M051_EVENT_CAPACITY];
static uint8_t qread, qwrite, qcount;
static psre_m051_status_t status;
static void enqueue(uint8_t port, psre_m051_event_type_t type, psre_m051_device_state_t state, uint32_t value) {
    if (qcount==PSRE_M051_EVENT_CAPACITY) { ++status.dropped_events; return; }
    queue[qwrite]=(psre_m051_event_t){port,type,state,value,psre_pit_ticks()};
    qwrite=(uint8_t)((qwrite+1u)%PSRE_M051_EVENT_CAPACITY); ++qcount; ++status.events;
    if(type==PSRE_M051_EVENT_CONNECT) ++status.connect_events;
    if(type==PSRE_M051_EVENT_DISCONNECT) ++status.disconnect_events;
}
bool psre_m051_initialize(void) {
    if(status.initialized) return true;
    status=(psre_m051_status_t){0};
    for(uint8_t i=0;i<=PSRE_M051_MAX_PORTS;i++) ports[i]=(port_state_t){0};
    qread=qwrite=qcount=0;
    const psre_m015_status_t *hc=psre_m015_get_status();
    if(hc && hc->state==PSRE_M015_STATE_RUNNING && hc->max_ports) {
        status.controller_available=true;
        status.port_count=hc->max_ports>PSRE_M051_MAX_PORTS?PSRE_M051_MAX_PORTS:hc->max_ports;
    }
    status.initialized=true;
    serial_write(status.controller_available ? "[M051] USB port lifecycle monitor initialized.\n" :
        "[M051] Lifecycle monitor initialized; xHCI unavailable, serial recovery retained.\n");
    return true;
}
void psre_m051_scan(void) {
    if(!status.initialized || !status.controller_available || status.scanning) return;
    status.scanning=true; ++status.scans; status.last_scan_tick=psre_pit_ticks();
    for(uint8_t p=1;p<=status.port_count;p++) {
        uint32_t value=0;
        if(!psre_m016_read_port_status(p,&value)) { ++status.read_errors; continue; }
        port_state_t *slot=&ports[p]; bool connected=(value&PORTSC_CCS)!=0;
        bool changed=!slot->known || slot->connected!=connected || ((value&PORTSC_CSC)!=0);
        if(changed) {
            slot->known=true; slot->connected=connected; slot->last_portsc=value;
            slot->state=connected?PSRE_M051_CONNECTED:PSRE_M051_DISCONNECTED;
            enqueue(p,connected?PSRE_M051_EVENT_CONNECT:PSRE_M051_EVENT_DISCONNECT,slot->state,value);
        }
    }
    status.scanning=false;
}
bool psre_m051_next_event(psre_m051_event_t *event) {
    if(!event || !qcount) return false;
    *event=queue[qread]; qread=(uint8_t)((qread+1u)%PSRE_M051_EVENT_CAPACITY); --qcount; return true;
}
psre_m051_device_state_t psre_m051_port_state(uint8_t port) {
    if(!port || port>status.port_count) return PSRE_M051_EMPTY;
    return ports[port].state;
}
const psre_m051_status_t *psre_m051_get_status(void) { return &status; }
void psre_m051_report(void) {
    serial_write("[M051] initialized="); serial_write(status.initialized?"yes":"no");
    serial_write(" xhci="); serial_write(status.controller_available?"ready":"not-ready");
    serial_write(" ports="); serial_write(status.port_count?"available":"0");
    serial_write(" scans="); serial_write(status.scans?"active":"0");
    serial_write(" events="); serial_write(status.events?"queued/recorded":"0");
    serial_write(" connect="); serial_write(status.connect_events?"yes":"0");
    serial_write(" disconnect="); serial_write(status.disconnect_events?"yes":"0");
    serial_write(" read_errors="); serial_write(status.read_errors?"yes":"0");
    serial_write(" dropped="); serial_write(status.dropped_events?"yes":"0"); serial_write("\n");
}
