#include <psre/m012/xhci_caps.h>
#include <psre/m011/xhci.h>
#include <psre/serial.h>
#include <stdint.h>

static psre_m012_xhci_caps_t last_caps;
static bool test_passed;

static uint16_t rd16(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}
static uint32_t rd32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

bool psre_m012_parse_capability_snapshot(const uint8_t *b, uint32_t n,
                                         psre_m012_xhci_caps_t *out) {
    if (!b || !out) return false;
    *out = (psre_m012_xhci_caps_t){0};
    /* Capability registers occupy offsets through RTSOFF (0x18..0x1b). */
    if (n < 0x1C) return false;

    uint8_t caplen = b[0];
    uint16_t version = rd16(&b[2]);
    uint32_t hcs1 = rd32(&b[4]);
    uint32_t hcc1 = rd32(&b[0x10]);
    uint32_t dboff = rd32(&b[0x14]);
    uint32_t rtsoff = rd32(&b[0x18]);

    if (caplen < 0x20 || (caplen & 0x3u) != 0) return false;
    if (version < 0x0100) return false;
    if ((dboff & 0x3u) != 0 || (rtsoff & 0x1Fu) != 0) return false;
    if ((dboff & 0xFFFFFFFCu) < caplen) return false;
    if ((rtsoff & 0xFFFFFFE0u) < caplen) return false;

    out->cap_length = caplen;
    out->interface_version = version;
    out->max_slots = (uint8_t)(hcs1 & 0xFFu);
    out->max_interrupters = (uint16_t)((hcs1 >> 8) & 0x7FFu);
    out->max_ports = (uint8_t)(hcs1 >> 24);
    out->doorbell_offset = dboff & 0xFFFFFFFCu;
    out->runtime_offset = rtsoff & 0xFFFFFFE0u;
    out->extended_capability_dword_offset = (uint16_t)((hcc1 >> 16) & 0xFFFFu);

    /* A controller with no slots, interrupters, or ports is not useful. */
    if (!out->max_slots || !out->max_interrupters || !out->max_ports) {
        *out = (psre_m012_xhci_caps_t){0};
        return false;
    }
    out->valid = true;
    return true;
}

bool psre_m012_software_selftest(void) {
    uint8_t sample[0x1C] = {0};
    psre_m012_xhci_caps_t parsed;
    /* xHCI 1.0, CAPLENGTH=0x40, 8 slots, 4 interrupters, 8 ports.
       DBOFF=0x1000 and RTSOFF=0x2000; xECP=0x40 (DWORD offset). */
    sample[0] = 0x40;
    sample[2] = 0x00; sample[3] = 0x01;
    sample[4] = 8;
    sample[5] = 4;
    sample[7] = 8;
    sample[0x12] = 0x40;
    sample[0x14] = 0x00; sample[0x15] = 0x10;
    sample[0x18] = 0x00; sample[0x19] = 0x20;

    bool valid = psre_m012_parse_capability_snapshot(sample, sizeof(sample), &parsed);
    bool ok = valid && parsed.cap_length == 0x40 &&
              parsed.interface_version == 0x0100 &&
              parsed.max_slots == 8 && parsed.max_interrupters == 4 &&
              parsed.max_ports == 8 && parsed.doorbell_offset == 0x1000 &&
              parsed.runtime_offset == 0x2000 &&
              parsed.extended_capability_dword_offset == 0x40;

    /* Reject misaligned CAPLENGTH and truncated snapshots. */
    sample[0] = 0x41;
    if (psre_m012_parse_capability_snapshot(sample, sizeof(sample), &parsed)) ok = false;
    sample[0] = 0x40;
    if (psre_m012_parse_capability_snapshot(sample, 0x18, &parsed)) ok = false;

    test_passed = ok;
    if (ok) last_caps = (psre_m012_xhci_caps_t){
        .valid=true, .cap_length=0x40, .interface_version=0x0100,
        .max_slots=8, .max_interrupters=4, .max_ports=8,
        .doorbell_offset=0x1000, .runtime_offset=0x2000,
        .extended_capability_dword_offset=0x40
    };
    serial_write(ok ? "[M012] Capability parser self-test PASS (synthetic data only).\n" :
                      "[M012] Capability parser self-test FAIL.\n");
    return ok;
}

void psre_m012_report(void) {
    serial_write("\n--- PSRE M012 xHCI CAPABILITY PARSER ---\n");
    serial_write(test_passed ? "[M012] Parser self-test: PASS\n" :
                               "[M012] Parser self-test: NOT RUN / FAIL\n");
    serial_write("[M012] Live MMIO snapshot: NOT AVAILABLE\n");
    serial_write("[M012] BAR mapping, PCI bus mastering, controller reset/start, "
                 "ports and USB enumeration remain DISABLED.\n");
}
