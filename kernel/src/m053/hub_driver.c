#include <psre/m053/hub_driver.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/m020/usb_device.h>
#include <psre/m052/usb_hub.h>
#include <psre/serial.h>
#include <stddef.h>

static psre_m053_status_t st;
static psre_m052_hub_descriptor_t descriptor;
static bool valid_port(uint8_t p) { return st.hub_probed && p && p <= st.port_count; }
static bool hub_control(uint8_t type, uint8_t request, uint16_t value,
                        uint16_t index, uint16_t length, uint8_t *data,
                        uint16_t capacity) {
    bool ok;
    if (length) ok = psre_m018_control_in(type, request, value, index, length, data, capacity);
    else ok = psre_m018_control_no_data(type, request, value, index);
    if (!ok) ++st.control_errors;
    return ok;
}
bool psre_m053_initialize(void) {
    if (st.initialized) return true;
    st = (psre_m053_status_t){0}; st.initialized = true;
    serial_write("[M053] USB hub class driver initialized; active EP0 transport required.\n");
    return true;
}
bool psre_m053_probe_active_hub(uint8_t hub_id, uint8_t parent_hub_id, uint8_t parent_port) {
    if (!st.initialized || !hub_id) return false;
    const psre_m018_status_t *dev = psre_m018_get_status();
    const psre_m020_status_t *cfg = psre_m020_get_status();
    if (!dev || !dev->address_assigned || !dev->descriptor_read || !cfg || !cfg->configured || cfg->hub_interfaces == 0u) {
        serial_write("[M053] Active M018 device is not a configured USB Hub.\n"); return false;
    }
    uint8_t head[7] = {0};
    ++st.probes;
    /* Class GET_DESCRIPTOR(HUB): bmRequestType=0xA0, bRequest=GET_DESCRIPTOR. */
    if (!hub_control(0xA0u, 6u, (uint16_t)(PSRE_M052_HUB_DESCRIPTOR_TYPE << 8), 0u, 7u, head, sizeof(head))) return false;
    if (head[0] < 9u || head[1] != PSRE_M052_HUB_DESCRIPTOR_TYPE) { ++st.control_errors; return false; }
    uint8_t full[64] = {0};
    uint8_t total = head[0];
    if (total > sizeof(full) || total < 9u ||
        !hub_control(0xA0u, 6u, (uint16_t)(PSRE_M052_HUB_DESCRIPTOR_TYPE << 8), 0u, total, full, sizeof(full))) return false;
    if (!psre_m052_parse_hub_descriptor(full, total, &descriptor)) { ++st.control_errors; return false; }
    if (!psre_m052_register_hub(hub_id, parent_hub_id, parent_port, descriptor.port_count, descriptor.characteristics)) return false;
    st.hub_id = hub_id; st.root_port = dev->port; st.port_count = descriptor.port_count;
    st.descriptor_valid = true; st.hub_probed = true;
    /* USB 2.0 hub class SET_FEATURE(PORT_POWER), selector 8. */
    for (uint8_t p = 1; p <= st.port_count; ++p) if (psre_m053_power_port(p)) ++st.ports_powered;
    st.power_requested = st.ports_powered != 0u;
    serial_write("[M053] Hub descriptor retrieved; downstream port power requested.\n");
    return true;
}
bool psre_m053_get_port_status(uint8_t port, psre_m053_port_status_t *out) {
    if (!valid_port(port) || !out) return false;
    uint8_t b[4] = {0};
    /* Class GET_STATUS(PORT): bmRequestType=0xA3, bRequest=0. */
    if (!hub_control(0xA3u, 0u, 0u, port, 4u, b, sizeof(b))) return false;
    out->status = (uint16_t)b[0] | ((uint16_t)b[1] << 8);
    out->change = (uint16_t)b[2] | ((uint16_t)b[3] << 8);
    ++st.status_reads; st.last_port = port;
    return true;
}
bool psre_m053_power_port(uint8_t port) {
    if (!st.hub_probed || !port || port > st.port_count) return false;
    /* Class SET_FEATURE(PORT_POWER), bmRequestType=0x23, bRequest=3. */
    return hub_control(0x23u, 3u, PSRE_M053_FEATURE_PORT_POWER, port, 0u, 0, 0u);
}
bool psre_m053_reset_port(uint8_t port) {
    if (!valid_port(port)) return false;
    /* Class SET_FEATURE(PORT_RESET). Reset completion is observed by a later scan. */
    return hub_control(0x23u, 3u, PSRE_M053_FEATURE_PORT_RESET, port, 0u, 0, 0u);
}
bool psre_m053_clear_port_change(uint8_t port, uint16_t feature) {
    if (!valid_port(port) || (feature != PSRE_M053_FEATURE_C_PORT_CONNECTION && feature != PSRE_M053_FEATURE_C_PORT_RESET)) return false;
    /* Class CLEAR_FEATURE(PORT change bit). */
    return hub_control(0x23u, 1u, feature, port, 0u, 0, 0u);
}
bool psre_m053_scan_ports(void) {
    if (!st.hub_probed) return false;
    bool all_ok = true;
    for (uint8_t p = 1; p <= st.port_count; ++p) {
        psre_m053_port_status_t ps;
        if (!psre_m053_get_port_status(p, &ps)) { all_ok = false; continue; }
        if (ps.change & 0x0001u) {
            ++st.connection_changes;
            uint8_t change_bitmap[2] = {0u, 0u};
            change_bitmap[p / 8u] = (uint8_t)(1u << (p % 8u));
            (void)psre_m052_process_change_bitmap(st.hub_id, change_bitmap, sizeof(change_bitmap));
            (void)psre_m053_clear_port_change(p, PSRE_M053_FEATURE_C_PORT_CONNECTION);
            /* A reset request is made only for a connected port; enumeration remains a later milestone. */
            if (ps.status & 0x0001u) (void)psre_m053_reset_port(p);
        }
        if (ps.change & 0x0010u) {
            ++st.reset_changes;
            (void)psre_m053_clear_port_change(p, PSRE_M053_FEATURE_C_PORT_RESET);
        }
    }
    return all_ok;
}
const psre_m053_status_t *psre_m053_get_status(void) { return &st; }
static void dec(uint32_t v) { char b[11]; unsigned n=0; if(!v){serial_write("0");return;} while(v&&n<sizeof(b)){b[n++]=(char)('0'+v%10u);v/=10u;} while(n)serial_write_char(b[--n]); }
void psre_m053_report(void) {
    serial_write("[M053] initialized="); serial_write(st.initialized?"yes":"no");
    serial_write(" probed="); serial_write(st.hub_probed?"yes":"no");
    serial_write(" hub_id="); dec(st.hub_id); serial_write(" ports="); dec(st.port_count);
    serial_write(" powered="); dec(st.ports_powered); serial_write(" scans_status_reads="); dec(st.status_reads);
    serial_write(" connection_changes="); dec(st.connection_changes); serial_write(" reset_changes="); dec(st.reset_changes);
    serial_write(" control_errors="); dec(st.control_errors); serial_write("\n");
}
