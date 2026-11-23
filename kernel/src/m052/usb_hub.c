#include <psre/m052/usb_hub.h>
#include <psre/serial.h>
#include <stddef.h>

static psre_m052_hub_t hubs[PSRE_M052_MAX_HUBS];
static psre_m052_device_t devices[PSRE_M052_MAX_DEVICES];
static psre_m052_event_t events[PSRE_M052_EVENT_CAPACITY];
static psre_m052_status_t status;
static uint8_t qread, qwrite, qcount;

static void enqueue(psre_m052_event_type_t type, uint8_t hub, uint8_t port, uint8_t device, uint32_t bitmap) {
    if (qcount == PSRE_M052_EVENT_CAPACITY) { ++status.dropped_events; return; }
    events[qwrite] = (psre_m052_event_t){type, hub, port, device, bitmap};
    qwrite = (uint8_t)((qwrite + 1u) % PSRE_M052_EVENT_CAPACITY); ++qcount;
}

bool psre_m052_parse_hub_descriptor(const uint8_t *data, uint16_t length, psre_m052_hub_descriptor_t *out) {
    if (!data || !out || length < 7u || data[0] < 9u || data[1] != PSRE_M052_HUB_DESCRIPTOR_TYPE) return false;
    uint8_t ports = data[2];
    if (!ports || ports > PSRE_M052_MAX_HUB_PORTS) return false;
    uint8_t bitmap_bytes = (uint8_t)(((uint16_t)ports + 1u + 7u) / 8u);
    uint16_t required = (uint16_t)(7u + 2u * bitmap_bytes);
    if (data[0] < required || length < data[0]) return false;
    out->port_count = ports;
    out->bitmap_bytes = bitmap_bytes;
    out->characteristics = (uint16_t)data[3] | ((uint16_t)data[4] << 8);
    out->power_good_2ms = data[5];
    out->controller_current_ma = data[6];
    return true;
}

bool psre_m052_initialize(void) {
    if (status.initialized) return true;
    for (uint8_t i=0;i<PSRE_M052_MAX_HUBS;i++) hubs[i]=(psre_m052_hub_t){0};
    for (uint8_t i=0;i<PSRE_M052_MAX_DEVICES;i++) devices[i]=(psre_m052_device_t){0};
    status=(psre_m052_status_t){0}; qread=qwrite=qcount=0; status.initialized=true;
    serial_write("[M052] USB hub topology and multi-device registry initialized (transport integration pending).\n");
    return true;
}

static const psre_m052_hub_t *find_hub(uint8_t id) {
    for (uint8_t i=0;i<PSRE_M052_MAX_HUBS;i++) if (hubs[i].present && hubs[i].id==id) return &hubs[i];
    return 0;
}
static psre_m052_hub_t *find_hub_mut(uint8_t id) {
    for (uint8_t i=0;i<PSRE_M052_MAX_HUBS;i++) if (hubs[i].present && hubs[i].id==id) return &hubs[i];
    return 0;
}
bool psre_m052_register_hub(uint8_t id, uint8_t parent, uint8_t parent_port, uint8_t ports, uint16_t characteristics) {
    if (!status.initialized || !id || !ports || ports>PSRE_M052_MAX_HUB_PORTS || find_hub(id)) return false;
    uint8_t depth=0;
    if (parent) {
        const psre_m052_hub_t *p=find_hub(parent);
        if (!p || !parent_port || parent_port>p->port_count || p->depth>=PSRE_M052_MAX_DEPTH) return false;
        depth=(uint8_t)(p->depth+1u);
    } else if (parent_port) return false;
    for (uint8_t i=0;i<PSRE_M052_MAX_HUBS;i++) if (!hubs[i].present) {
        hubs[i]=(psre_m052_hub_t){true,id,parent,parent_port,depth,ports,characteristics,0};
        ++status.hub_count; enqueue(PSRE_M052_EVENT_HUB_REGISTERED,id,0,0,0); return true;
    }
    return false;
}

bool psre_m052_process_change_bitmap(uint8_t id, const uint8_t *bitmap, uint16_t length) {
    psre_m052_hub_t *hub=find_hub_mut(id);
    if (!hub || !bitmap) return false;
    uint16_t required=(uint16_t)(((uint16_t)hub->port_count+1u+7u)/8u);
    if (length<required) return false;
    for (uint8_t port=1;port<=hub->port_count;port++) {
        uint16_t bit=port;
        if (bitmap[bit/8u] & (uint8_t)(1u<<(bit%8u))) {
            ++hub->port_change_count; ++status.port_events;
            enqueue(PSRE_M052_EVENT_PORT_CHANGE,id,port,0,bitmap[bit/8u]);
        }
    }
    return true;
}

bool psre_m052_add_device(uint8_t id, uint8_t hub_id, uint8_t port, uint8_t slot, uint8_t root_port, uint16_t vid, uint16_t pid) {
    if (!status.initialized || !id || !slot || !root_port) return false;
    uint8_t depth=0;
    if (hub_id) {
        const psre_m052_hub_t *hub=find_hub(hub_id);
        if (!hub || !port || port>hub->port_count) return false;
        depth=(uint8_t)(hub->depth+1u);
    } else if (port) return false;
    for (uint8_t i=0;i<PSRE_M052_MAX_DEVICES;i++) {
        if (devices[i].present && devices[i].id==id) return false;
    }
    for (uint8_t i=0;i<PSRE_M052_MAX_DEVICES;i++) if (!devices[i].present || devices[i].state==PSRE_M052_DEV_REMOVED) {
        devices[i]=(psre_m052_device_t){true,id,hub_id,port,depth,slot,root_port,vid,pid,PSRE_M052_DEV_ACTIVE};
        ++status.device_count; ++status.device_adds; enqueue(PSRE_M052_EVENT_DEVICE_ADDED,hub_id,port,id,0); return true;
    }
    return false;
}

bool psre_m052_remove_device(uint8_t id) {
    for (uint8_t i=0;i<PSRE_M052_MAX_DEVICES;i++) if (devices[i].present && devices[i].id==id && devices[i].state!=PSRE_M052_DEV_REMOVED) {
        devices[i].state=PSRE_M052_DEV_REMOVED; if(status.device_count) --status.device_count;
        ++status.device_removes; enqueue(PSRE_M052_EVENT_DEVICE_REMOVED,devices[i].parent_hub_id,devices[i].parent_port,id,0); return true;
    }
    return false;
}
bool psre_m052_next_event(psre_m052_event_t *event) {
    if (!event || !qcount) return false;
    *event=events[qread]; qread=(uint8_t)((qread+1u)%PSRE_M052_EVENT_CAPACITY); --qcount; return true;
}
const psre_m052_hub_t *psre_m052_get_hub(uint8_t id) { return find_hub(id); }
const psre_m052_device_t *psre_m052_get_device(uint8_t id) {
    for (uint8_t i=0;i<PSRE_M052_MAX_DEVICES;i++) if (devices[i].present && devices[i].id==id) return &devices[i];
    return 0;
}
const psre_m052_status_t *psre_m052_get_status(void) { return &status; }
void psre_m052_report(void) {
    serial_write("[M052] initialized="); serial_write(status.initialized?"yes":"no");
    serial_write(" hubs="); serial_write(status.hub_count?"registered":"0");
    serial_write(" devices="); serial_write(status.device_count?"registered":"0");
    serial_write(" port_events="); serial_write(status.port_events?"yes":"0");
    serial_write(" descriptor_errors="); serial_write(status.descriptor_errors?"yes":"0");
    serial_write(" dropped_events="); serial_write(status.dropped_events?"yes":"0"); serial_write("\n");
}
