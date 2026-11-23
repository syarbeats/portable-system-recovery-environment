#include <psre/m014/xhci_live.h>
#include <psre/m004/pci.h>
#include <psre/m012/xhci_caps.h>
#include <psre/vmm.h>
#include <psre/serial.h>
#include <stdint.h>

#define XHCI_CAP_SNAPSHOT_SIZE 0x1Cu
#define XHCI_BAR_PROBE_MAP_SIZE 0x20u

static psre_m014_xhci_live_status_t live;

static uint32_t mmio_read32(const volatile uint32_t *base, uint32_t offset) {
    return base[offset / sizeof(uint32_t)];
}

static bool find_xhci_bar(void) {
    const psre_pci_stats_t *stats = psre_pci_get_stats();
    const psre_pci_device_t *devices = psre_pci_get_devices();
    if (!stats || !devices || !stats->config_mechanism_1_available) return false;

    for (uint32_t i = 0; i < stats->stored; ++i) {
        const psre_pci_device_t *d = &devices[i];
        if (d->class_code != 0x0C || d->subclass != 0x03 ||
            d->prog_if != 0x30) continue;

        live.controller_found = true;
        live.bus = d->bus; live.device = d->device; live.function = d->function;
        live.vendor_id = d->vendor_id; live.device_id = d->device_id;

        for (uint8_t b = 0; b < d->bar_count; ++b) {
            const psre_pci_bar_t *bar = &d->bars[b];
            if (bar->kind != 2 || !bar->address) continue;
            /* Reject non-64-bit BARs above 4 GiB only implicitly; the MMIO
               mapper accepts physical addresses supported by its page tables. */
            live.bar_index = bar->index;
            live.bar_address = bar->address;
            live.bar_valid = (bar->address & 0xFFFu) == 0;
            return live.bar_valid;
        }
        return false;
    }
    return false;
}

bool psre_m014_probe_xhci(void) {
    live = (psre_m014_xhci_live_status_t){0};
    if (!find_xhci_bar()) {
        serial_write("[M014] xHCI controller/MMIO BAR not found or invalid.\n");
        return false;
    }

    /* Map only the first 32 bytes needed for the read-only capability snapshot.
       M013 maps a whole page internally, but this module dereferences no other
       register offsets and never probes BAR sizing by writing all-ones. */
    volatile uint32_t *bar = (volatile uint32_t *)
        vmm_map_mmio(live.bar_address, XHCI_BAR_PROBE_MAP_SIZE);
    if (!bar) {
        serial_write("[M014] MMIO mapping rejected; BAR left untouched.\n");
        return false;
    }
    live.mmio_mapped = true;

    uint8_t snapshot[XHCI_CAP_SNAPSHOT_SIZE];
    for (uint32_t off = 0; off < XHCI_CAP_SNAPSHOT_SIZE; off += 4) {
        uint32_t value = mmio_read32(bar, off);
        snapshot[off] = (uint8_t)value;
        snapshot[off + 1] = (uint8_t)(value >> 8);
        snapshot[off + 2] = (uint8_t)(value >> 16);
        snapshot[off + 3] = (uint8_t)(value >> 24);
    }

    psre_m012_xhci_caps_t caps;
    bool parsed = psre_m012_parse_capability_snapshot(
        snapshot, sizeof(snapshot), &caps);

    /* Always release the temporary MMIO mapping before returning. */
    bool unmapped = vmm_unmap_mmio((void *)bar, XHCI_BAR_PROBE_MAP_SIZE);
    live.mmio_mapped = !unmapped;
    live.mmio_active_after_probe = vmm_mmio_active_mappings();

    if (!unmapped) {
        serial_write("[M014] WARNING: MMIO mapping could not be released.\n");
        return false;
    }
    if (!parsed) {
        serial_write("[M014] Live capability snapshot rejected by M012 parser.\n");
        return false;
    }

    live.capability_valid = true;
    live.cap_length = caps.cap_length;
    live.interface_version = caps.interface_version;
    live.max_slots = caps.max_slots;
    live.max_interrupters = caps.max_interrupters;
    live.max_ports = caps.max_ports;
    live.doorbell_offset = caps.doorbell_offset;
    live.runtime_offset = caps.runtime_offset;
    live.xecp_dword_offset = caps.extended_capability_dword_offset;
    serial_write("[M014] Live xHCI capability probe PASS (read-only).\n");
    return true;
}

void psre_m014_initialize(void) {
    serial_write("\n=== PSRE M014 LIVE xHCI MMIO DISCOVERY ===\n");
    (void)psre_m014_probe_xhci();
    psre_m014_report();
}

const psre_m014_xhci_live_status_t *psre_m014_get_status(void) {
    return &live;
}

static void hex(uint64_t v, unsigned n) {
    static const char h[] = "0123456789ABCDEF";
    for (unsigned i = 0; i < n; ++i)
        serial_write_char(h[(v >> ((n - 1u - i) * 4u)) & 15u]);
}
static void decimal(uint32_t v) {
    char b[10]; unsigned n = 0;
    if (!v) { serial_write("0"); return; }
    while (v && n < sizeof(b)) { b[n++] = (char)('0' + v % 10u); v /= 10u; }
    while (n) serial_write_char(b[--n]);
}

void psre_m014_report(void) {
    serial_write("\n--- PSRE M014 LIVE xHCI STATUS ---\n");
    if (!live.controller_found) {
        serial_write("[M014] No xHCI controller discovered.\n");
        return;
    }
    serial_write("[M014] PCI "); hex(live.bus, 2); serial_write(":");
    hex(live.device, 2); serial_write("."); hex(live.function, 1);
    serial_write(" VID:DID="); hex(live.vendor_id, 4); serial_write(":");
    hex(live.device_id, 4); serial_write("\n");
    serial_write("[M014] BAR"); hex(live.bar_index, 1);
    serial_write(" physical=0x"); hex(live.bar_address, 16);
    serial_write(live.bar_valid ? " VALID\n" : " INVALID\n");
    if (live.capability_valid) {
        serial_write("[M014] xHCI version=0x"); hex(live.interface_version, 4);
        serial_write(" CAPLENGTH=0x"); hex(live.cap_length, 2);
        serial_write("\n[M014] Slots="); decimal(live.max_slots);
        serial_write(" Interrupters="); decimal(live.max_interrupters);
        serial_write(" Ports="); decimal(live.max_ports); serial_write("\n");
        serial_write("[M014] DBOFF=0x"); hex(live.doorbell_offset, 8);
        serial_write(" RTSOFF=0x"); hex(live.runtime_offset, 8);
        serial_write(" xECP DWORD offset=0x"); hex(live.xecp_dword_offset, 4);
        serial_write("\n");
    } else {
        serial_write("[M014] Live capability data unavailable/invalid.\n");
    }
    serial_write("[M014] MMIO mappings remaining="); decimal(live.mmio_active_after_probe);
    serial_write("\n[M014] READ-ONLY: no PCI command writes, BIOS handoff, "
                 "controller halt/reset/start, doorbell, or USB enumeration.\n");
}
