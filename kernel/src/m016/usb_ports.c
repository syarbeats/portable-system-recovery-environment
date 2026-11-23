#include <psre/m016/usb_ports.h>
#include <psre/m015/xhci_init.h>
#include <psre/m004/pci.h>
#include <psre/m003/pit.h>
#include <psre/vmm.h>
#include <psre/serial.h>
#include <stdint.h>
#include <stdbool.h>

#define M016_MAX_MMIO_BYTES       0x10000u
#define M016_PORT_REG_BASE        0x400u
#define M016_PORT_REG_STRIDE      0x10u
#define M016_TIMEOUT_TICKS        500u
#define M016_POWER_WAIT_TICKS     10u
#define M016_TIMEOUT_SPINS        50000000u

#define PORTSC_CCS                (1u << 0)
#define PORTSC_PED                (1u << 1)
#define PORTSC_OCA                (1u << 3)
#define PORTSC_PR                 (1u << 4)
#define PORTSC_PLS_MASK           (0xFu << 5)
#define PORTSC_PP                 (1u << 9)
#define PORTSC_SPEED_MASK         (0xFu << 10)
#define PORTSC_PIC_MASK           (3u << 14)
#define PORTSC_LWS                (1u << 16)
#define PORTSC_CSC                (1u << 17)
#define PORTSC_PEC                (1u << 18)
#define PORTSC_WRC                (1u << 19)
#define PORTSC_OCC                (1u << 20)
#define PORTSC_PRC                (1u << 21)
#define PORTSC_PLC                (1u << 22)
#define PORTSC_CEC                (1u << 23)
#define PORTSC_WCE                (1u << 25)
#define PORTSC_WDE                (1u << 26)
#define PORTSC_WOE                (1u << 27)
#define PORTSC_WPR                (1u << 31)

#define PORTSC_CHANGE_MASK        (PORTSC_CSC | PORTSC_PEC | PORTSC_WRC | \
                                   PORTSC_OCC | PORTSC_PRC | PORTSC_PLC | \
                                   PORTSC_CEC)
#define PORTSC_CONTROL_MASK       (PORTSC_PP | PORTSC_PIC_MASK | \
                                   PORTSC_WCE | PORTSC_WDE | PORTSC_WOE)

#define HCC_PPC                  (1u << 3)

static psre_m016_status_t status;
static volatile uint32_t *mapped_bar;
static uint32_t mapped_size;
static uint32_t operational_base;
static bool port_power_control;

static uint32_t mmio_read32(uint32_t offset) {
    return mapped_bar[offset / 4u];
}
static void mmio_write32(uint32_t offset, uint32_t value) {
    mapped_bar[offset / 4u] = value;
    __asm__ volatile("mfence" ::: "memory");
}
static uint32_t port_offset(uint8_t port) {
    return operational_base + M016_PORT_REG_BASE +
           ((uint32_t)(port - 1u) * M016_PORT_REG_STRIDE);
}
static const char *speed_name(uint8_t speed) {
    switch (speed) {
        case 1: return "Full-Speed";
        case 2: return "Low-Speed";
        case 3: return "High-Speed";
        case 4: return "SuperSpeed";
        case 5: return "SuperSpeedPlus";
        default: return "Unknown";
    }
}
static const char *link_name(uint8_t pls) {
    switch (pls) {
        case 0: return "U0";
        case 1: return "U1";
        case 2: return "U2";
        case 3: return "U3";
        case 4: return "Disabled";
        case 5: return "RxDetect";
        case 6: return "Inactive";
        case 7: return "Polling";
        case 8: return "Recovery";
        case 9: return "HotReset";
        case 10: return "Compliance";
        case 11: return "Test";
        case 15: return "Resume";
        default: return "Reserved";
    }
}
static void print_u32(uint32_t value) {
    char buffer[10];
    unsigned n = 0;
    if (!value) { serial_write("0"); return; }
    while (value && n < sizeof(buffer)) {
        buffer[n++] = (char)('0' + value % 10u);
        value /= 10u;
    }
    while (n) serial_write_char(buffer[--n]);
}
static void print_hex32(uint32_t value) {
    static const char digits[] = "0123456789ABCDEF";
    for (int i = 7; i >= 0; --i)
        serial_write_char(digits[(value >> (i * 4)) & 0xFu]);
}
static bool wait_ticks(uint64_t ticks, uint32_t *stage) {
    uint64_t start = psre_pit_ticks();
    for (uint64_t spins = 0; spins < M016_TIMEOUT_SPINS; ++spins) {
        if (psre_pit_ticks() - start >= ticks) return true;
        __asm__ volatile("pause");
    }
    if (stage) *stage = 1;
    return false;
}
static bool controller_is_ready(void) {
    const psre_m015_status_t *m15 = psre_m015_get_status();
    return m15 && m15->state == PSRE_M015_STATE_RUNNING &&
           m15->controller_running && m15->busmaster_enabled &&
           m15->controller_found && m15->max_ports != 0;
}
static bool map_controller(void) {
    status.controller_ready = false;
    if (mapped_bar) {
        serial_write("[M016] A previous MMIO mapping is still active; refusing another map.\n");
        return false;
    }
    const psre_m015_status_t *m15 = psre_m015_get_status();
    if (!controller_is_ready() || !m15) {
        serial_write("[M016] M015 controller state is not RUNNING; execute m015-init first.\n");
        return false;
    }

    uint64_t required = (uint64_t)m15->cap_length + M016_PORT_REG_BASE +
                       ((uint64_t)m15->max_ports * M016_PORT_REG_STRIDE);
    if (required > M016_MAX_MMIO_BYTES || required < M016_PORT_REG_BASE) {
        serial_write("[M016] Port register window exceeds 64 KiB safety bound.\n");
        return false;
    }
    mapped_size = (uint32_t)((required + 4095u) & ~4095u);
    mapped_bar = (volatile uint32_t *)vmm_map_mmio(m15->bar_address, mapped_size);
    if (!mapped_bar) {
        serial_write("[M016] Cannot map xHCI BAR; no port register was modified.\n");
        return false;
    }
    operational_base = m15->cap_length;
    uint32_t hcc1 = mmio_read32(0x10u);
    port_power_control = (hcc1 & HCC_PPC) != 0;
    status.mmio_mapped = true;
    status.controller_ready = true;
    status.port_count = m15->max_ports;
    return true;
}
static bool unmap_controller(void) {
    bool ok = true;
    if (mapped_bar) {
        ok = vmm_unmap_mmio((void *)mapped_bar, mapped_size);
        if (ok) {
            mapped_bar = 0;
            mapped_size = 0;
        }
    }
    status.mmio_mapped = !ok;
    return ok;
}
static uint32_t port_control_bits(uint32_t portsc) {
    return portsc & PORTSC_CONTROL_MASK;
}
static void clear_port_change(uint32_t offset, uint32_t portsc,
                              uint32_t change_bit) {
    /* PORTSC change bits are RW1C. Preserve only RW control fields; never
       echo read-only state or unrelated RW1C flags back to the register. */
    mmio_write32(offset, port_control_bits(portsc) | change_bit);
}
static void report_port(uint8_t port, uint32_t portsc) {
    uint8_t speed = (uint8_t)((portsc & PORTSC_SPEED_MASK) >> 10);
    uint8_t pls = (uint8_t)((portsc & PORTSC_PLS_MASK) >> 5);
    serial_write("[M016] Port "); print_u32(port); serial_write(": ");
    serial_write((portsc & PORTSC_CCS) ? "CONNECTED" : "DISCONNECTED");
    serial_write(" | "); serial_write(speed_name(speed));
    serial_write(" | Link="); serial_write(link_name(pls));
    serial_write(" | Power=");
    if (!port_power_control) serial_write("N/A");
    else serial_write((portsc & PORTSC_PP) ? "ON" : "OFF");
    serial_write(" | Enable="); serial_write((portsc & PORTSC_PED) ? "YES" : "NO");
    serial_write(" | OverCurrent="); serial_write((portsc & PORTSC_OCA) ? "YES" : "NO");
    serial_write(" | Change=0x"); print_hex32(portsc & PORTSC_CHANGE_MASK);
    serial_write("\n");
}
bool psre_m016_scan_ports(void) {
    status.last_scanned_port = 0;
    if (!map_controller()) return false;

    serial_write("\n--- PSRE M016 USB PORT INVENTORY ---\n");
    serial_write("[M016] Ports reported by HCSPARAMS1: ");
    print_u32(status.port_count);
    serial_write("\n[M016] Per-port power control: ");
    serial_write(port_power_control ? "supported\n" : "not advertised\n");

    for (uint16_t p = 1; p <= status.port_count; ++p) {
        uint32_t offset = port_offset((uint8_t)p);
        uint32_t value = mmio_read32(offset);
        report_port((uint8_t)p, value);
        status.last_scanned_port = (uint8_t)p;
        status.last_portsc = value;
    }

    if (!unmap_controller()) {
        serial_write("[M016] WARNING: MMIO mapping could not be released.\n");
        return false;
    }
    return true;
}
bool psre_m016_reset_port(uint8_t port) {
    status.last_reset_port = port;
    status.last_reset_success = false;
    status.last_speed_id = 0;
    status.last_timeout_stage = 0;

    if (!map_controller()) return false;
    if (!port || port > status.port_count) {
        serial_write("[M016] Invalid port number. Valid range: 1..");
        print_u32(status.port_count);
        serial_write("\n");
        (void)unmap_controller();
        return false;
    }

    uint32_t offset = port_offset(port);
    uint32_t initial = mmio_read32(offset);
    status.last_portsc = initial;

    if (!(initial & PORTSC_CCS)) {
        serial_write("[M016] Reset rejected: no device connected to this port.\n");
        (void)unmap_controller();
        return false;
    }
    if (initial & PORTSC_OCA) {
        serial_write("[M016] Reset rejected: port reports over-current.\n");
        (void)unmap_controller();
        return false;
    }

    uint8_t speed = (uint8_t)((initial & PORTSC_SPEED_MASK) >> 10);
    if (speed == 0 || speed > 5) {
        serial_write("[M016] Reset rejected: unsupported/unknown speed ID.\n");
        (void)unmap_controller();
        return false;
    }
    status.last_speed_id = speed;

    uint32_t controls = port_control_bits(initial);
    if (port_power_control && !(initial & PORTSC_PP)) {
        mmio_write32(offset, controls | PORTSC_PP);
        if (!wait_ticks(M016_POWER_WAIT_TICKS, &status.last_timeout_stage)) {
            serial_write("[M016] Timed out waiting for port power stabilization.\n");
            (void)unmap_controller();
            return false;
        }
        initial = mmio_read32(offset);
        if (!(initial & PORTSC_CCS) || (initial & PORTSC_OCA)) {
            serial_write("[M016] Connection/over-current changed during power-up; reset cancelled.\n");
            (void)unmap_controller();
            return false;
        }
        controls = port_control_bits(initial);
    }

    bool superspeed = speed >= 4;
    uint32_t reset_bit = superspeed ? PORTSC_WPR : PORTSC_PR;
    uint32_t change_bit = superspeed ? PORTSC_WRC : PORTSC_PRC;

    /* Clear only the corresponding stale RW1C completion flag. */
    uint32_t before = mmio_read32(offset);
    if (before & change_bit) clear_port_change(offset, before, change_bit);

    serial_write("[M016] Resetting port "); print_u32(port);
    serial_write(superspeed ? " using Warm Port Reset (WPR).\n" :
                              " using Port Reset (PR).\n");
    mmio_write32(offset, controls | reset_bit);

    uint64_t start = psre_pit_ticks();
    bool complete = false;
    for (uint64_t spins = 0; spins < M016_TIMEOUT_SPINS; ++spins) {
        uint32_t value = mmio_read32(offset);
        status.last_portsc = value;
        if (!(value & reset_bit) && (value & change_bit)) {
            complete = true;
            break;
        }
        if (psre_pit_ticks() - start >= M016_TIMEOUT_TICKS) break;
        __asm__ volatile("pause");
    }

    if (!complete) {
        status.last_timeout_stage = superspeed ? 3u : 2u;
        serial_write("[M016] Port reset timed out; inspect PORTSC and controller state.\n");
        (void)unmap_controller();
        return false;
    }

    uint32_t after = mmio_read32(offset);
    clear_port_change(offset, after, change_bit);
    after = mmio_read32(offset);
    status.last_portsc = after;

    if (!(after & PORTSC_CCS)) {
        serial_write("[M016] Device disconnected during reset.\n");
        (void)unmap_controller();
        return false;
    }

    report_port(port, after);
    if (after & PORTSC_PED) {
        status.last_reset_success = true;
        serial_write("[M016] Port reset completed; port is enabled.\n");
    } else {
        serial_write("[M016] Reset completed, but PED is not set. Enumeration cannot proceed yet.\n");
    }

    if (!unmap_controller()) {
        serial_write("[M016] WARNING: MMIO mapping could not be released.\n");
        return false;
    }
    return status.last_reset_success;
}

bool psre_m016_read_port_status(uint8_t port, uint32_t *portsc) {
    if (!portsc || !map_controller()) return false;
    if (!port || port > status.port_count) {
        (void)unmap_controller();
        return false;
    }
    uint32_t value = mmio_read32(port_offset(port));
    *portsc = value;
    status.last_scanned_port = port;
    status.last_portsc = value;
    return unmap_controller();
}

const psre_m016_status_t *psre_m016_get_status(void) { return &status; }

void psre_m016_report(void) {
    serial_write("\n--- PSRE M016 USB PORT STATUS ---\n");
    serial_write("[M016] Controller ready: ");
    serial_write(status.controller_ready ? "yes\n" : "no\n");
    serial_write("[M016] Port count: "); print_u32(status.port_count);
    serial_write("\n[M016] Last scanned port: "); print_u32(status.last_scanned_port);
    serial_write("\n[M016] Last reset port: "); print_u32(status.last_reset_port);
    serial_write("\n[M016] Last speed ID: "); print_u32(status.last_speed_id);
    serial_write("\n[M016] Last reset result: ");
    serial_write(status.last_reset_success ? "SUCCESS\n" : "not successful/not run\n");
    serial_write("[M016] Last PORTSC: 0x"); print_hex32(status.last_portsc);
    serial_write("\n[M016] Timeout stage: "); print_u32(status.last_timeout_stage);
    serial_write("\n[M016] MMIO mapping active: ");
    serial_write(status.mmio_mapped ? "yes\n" : "no\n");
    serial_write("[M016] Scope: port inventory and explicit reset only; no Enable Slot, "
                 "device enumeration, descriptor requests, or USB data transfers.\n");
}
