#include <psre/m015/xhci_init.h>
#include <psre/m014/xhci_live.h>
#include <psre/m004/pci.h>
#include <psre/pmm.h>
#include <psre/memory_map.h>
#include <psre/m003/pit.h>
#include <psre/vmm.h>
#include <psre/serial.h>
#include <stdint.h>
#include <stdbool.h>

#define XHCI_MAP_BYTES          0x10000u
#define XHCI_MAX_SCRATCHPADS    1023u
#define XHCI_RING_TRBS          256u
#define XHCI_DMA_LIMIT          0xFFFFFFFFULL
#define XHCI_TIMEOUT_TICKS      500u
#define XHCI_TIMEOUT_SPINS      50000000u

#define CAP_HCSPARAMS1          0x04u
#define CAP_HCSPARAMS2          0x08u
#define CAP_HCCPARAMS1          0x10u
#define CAP_DBOFF               0x14u
#define CAP_RTSOFF              0x18u

#define USBCMD_RS               (1u << 0)
#define USBCMD_HCRST            (1u << 1)
#define USBCMD_INTE             (1u << 2)
#define USBSTS_HCH              (1u << 0)
#define USBSTS_CNR              (1u << 11)

#define XHCI_TRB_TYPE_SHIFT     10u
#define XHCI_TRB_TYPE_LINK      6u
#define XHCI_TRB_CYCLE          1u

typedef struct __attribute__((packed, aligned(16))) {
    uint64_t parameter;
    uint32_t status;
    uint32_t control;
} xhci_trb_t;

typedef struct __attribute__((packed)) {
    uint64_t segment_base;
    uint32_t segment_size;
    uint32_t reserved;
} xhci_erst_entry_t;

static psre_m015_status_t st;
static volatile uint32_t *mmio;
static uint64_t dma_ring_phys, dcbaa_phys, scratch_array_phys;
static uint64_t scratch_phys[XHCI_MAX_SCRATCHPADS];
static uint32_t scratch_allocated;
static uint64_t ring_pages = 3, dcbaa_pages, scratch_array_pages;
static xhci_trb_t *command_ring, *event_ring;
static xhci_erst_entry_t *erst;
static uint64_t *dcbaa, *scratch_array;
static uint8_t *ring_virt, *dcbaa_virt, *scratch_array_virt;
static uint32_t mapped_bytes;
static uint16_t command_enqueue;
static uint8_t command_cycle = 1;
static uint16_t event_dequeue;
static uint8_t event_cycle = 1;

static uint32_t rd32(uint32_t off) {
    return mmio[off / 4u];
}
static void wr32(uint32_t off, uint32_t value) {
    mmio[off / 4u] = value;
    __asm__ volatile("mfence" ::: "memory");
}
static void zero_bytes(void *ptr, uint64_t size) {
    uint8_t *b = (uint8_t *)ptr;
    for (uint64_t i = 0; i < size; ++i) b[i] = 0;
}
static bool wait_mask(uint32_t offset, uint32_t mask, uint32_t expected,
                      uint32_t stage) {
    uint64_t start = psre_pit_ticks();
    for (uint64_t spins = 0; spins < XHCI_TIMEOUT_SPINS; ++spins) {
        if ((rd32(offset) & mask) == expected) return true;
        if (psre_pit_ticks() - start >= XHCI_TIMEOUT_TICKS) break;
        __asm__ volatile("pause");
    }
    st.timeout_stage = stage;
    return false;
}
static bool wait_reg_clear(uint32_t offset, uint32_t mask, uint32_t stage) {
    return wait_mask(offset, mask, 0, stage);
}
static void release_dma(void) {
    if (scratch_allocated) {
        for (uint32_t i = 0; i < scratch_allocated; ++i)
            (void)pmm_free_page(scratch_phys[i]);
    }
    scratch_allocated = 0;
    if (scratch_array_phys && scratch_array_pages)
        (void)pmm_free_contiguous_pages(scratch_array_phys, scratch_array_pages);
    if (dcbaa_phys && dcbaa_pages)
        (void)pmm_free_contiguous_pages(dcbaa_phys, dcbaa_pages);
    if (dma_ring_phys && ring_pages)
        (void)pmm_free_contiguous_pages(dma_ring_phys, ring_pages);
    scratch_array_phys = dcbaa_phys = dma_ring_phys = 0;
    scratch_array_pages = dcbaa_pages = 0;
    scratch_array = dcbaa = 0;
    scratch_array_virt = dcbaa_virt = ring_virt = 0;
    command_ring = event_ring = 0;
    erst = 0;
    st.dma_ready = false;
}
static bool alloc_dma_block(uint64_t pages, uint64_t *phys, uint8_t **virt) {
    if (!pages || !phys || !virt) return false;
    uint64_t address = pmm_alloc_contiguous_pages(pages, XHCI_DMA_LIMIT);
    if (!address) return false;
    uint64_t hhdm = memory_map_hhdm_offset();
    if (!hhdm || address > UINT64_MAX - hhdm) {
        (void)pmm_free_contiguous_pages(address, pages);
        return false;
    }
    *phys = address;
    *virt = (uint8_t *)(uintptr_t)(hhdm + address);
    zero_bytes(*virt, pages * PSRE_PAGE_SIZE);
    return true;
}
static bool alloc_dma(void) {
    dcbaa_pages = 1; /* MaxSlots is an 8-bit field: (255+1)*8 fits in 4 KiB. */
    if (!alloc_dma_block(ring_pages, &dma_ring_phys, &ring_virt)) return false;
    if (!alloc_dma_block(dcbaa_pages, &dcbaa_phys, &dcbaa_virt)) return false;

    uint32_t sp_count = st.scratchpad_count;
    if (sp_count > XHCI_MAX_SCRATCHPADS) return false;
    if (sp_count) {
        uint64_t bytes = (uint64_t)sp_count * sizeof(uint64_t);
        scratch_array_pages = (bytes + PSRE_PAGE_SIZE - 1u) / PSRE_PAGE_SIZE;
        if (!alloc_dma_block(scratch_array_pages, &scratch_array_phys,
                             &scratch_array_virt)) return false;
        scratch_array = (uint64_t *)scratch_array_virt;
        for (uint32_t i = 0; i < sp_count; ++i) {
            uint64_t page = pmm_alloc_contiguous_pages(1, XHCI_DMA_LIMIT);
            if (!page) return false;
            scratch_phys[scratch_allocated++] = page;
            scratch_array[i] = page;
            uint64_t hhdm = memory_map_hhdm_offset();
            if (!hhdm || page > UINT64_MAX - hhdm) return false;
            zero_bytes((void *)(uintptr_t)(hhdm + page), PSRE_PAGE_SIZE);
        }
    }

    dcbaa = (uint64_t *)dcbaa_virt;
    if (sp_count) dcbaa[0] = scratch_array_phys;
    command_ring = (xhci_trb_t *)ring_virt;
    event_ring = (xhci_trb_t *)(ring_virt + PSRE_PAGE_SIZE);
    erst = (xhci_erst_entry_t *)(ring_virt + 2u * PSRE_PAGE_SIZE);

    command_ring[XHCI_RING_TRBS - 1u].parameter = dma_ring_phys;
    command_ring[XHCI_RING_TRBS - 1u].status = 0;
    command_ring[XHCI_RING_TRBS - 1u].control =
        (XHCI_TRB_TYPE_LINK << XHCI_TRB_TYPE_SHIFT) | (1u << 1) |
        XHCI_TRB_CYCLE;
    command_enqueue = 0; command_cycle = 1;
    event_dequeue = 0; event_cycle = 1;

    erst[0].segment_base = dma_ring_phys + PSRE_PAGE_SIZE;
    erst[0].segment_size = XHCI_RING_TRBS;
    erst[0].reserved = 0;
    __asm__ volatile("mfence" ::: "memory");
    st.dma_ready = true;
    return true;
}
static bool find_controller(void) {
    const psre_m014_xhci_live_status_t *m14 = psre_m014_get_status();
    if (!m14 || !m14->controller_found || !m14->bar_valid ||
        !m14->capability_valid) return false;
    st.controller_found = true;
    st.bus = m14->bus; st.device = m14->device; st.function = m14->function;
    st.vendor_id = m14->vendor_id; st.device_id = m14->device_id;
    st.bar_address = m14->bar_address;
    st.max_slots = m14->max_slots;
    st.max_ports = m14->max_ports;
    st.max_interrupters = m14->max_interrupters;
    st.cap_length = m14->cap_length;
    st.runtime_offset = m14->runtime_offset;
    st.doorbell_offset = m14->doorbell_offset;
    return true;
}
static bool enable_pci_memory(void) {
    uint16_t command;
    if (!psre_pci_config_read16(st.bus, st.device, st.function, 0x04, &command))
        return false;
    if (!(command & (1u << 1))) {
        if (!psre_pci_config_write16(st.bus, st.device, st.function, 0x04,
                                     (uint16_t)(command | (1u << 1))))
            return false;
    }
    return true;
}
static bool enable_pci_busmaster(void) {
    uint16_t command;
    if (!psre_pci_config_read16(st.bus, st.device, st.function, 0x04, &command))
        return false;
    if (!psre_pci_config_write16(st.bus, st.device, st.function, 0x04,
                                 (uint16_t)(command | (1u << 1) | (1u << 2))))
        return false;
    st.busmaster_enabled = true;
    return true;
}
static bool bios_handoff(void) {
    uint32_t xecp = 0;
    /* HCCPARAMS1 is cached by M014 only as a DWORD offset. */
    const psre_m014_xhci_live_status_t *m14 = psre_m014_get_status();
    if (!m14 || !m14->xecp_dword_offset) {
        st.bios_handoff_done = true;
        return true;
    }
    xecp = (uint32_t)m14->xecp_dword_offset * 4u;
    if (xecp && (xecp < 0x20u || xecp > XHCI_MAP_BYTES - 4u)) return false;
    for (uint32_t n = 0; n < 64; ++n) {
        if (xecp < 0x20u || xecp > mapped_bytes - 4u) return false;
        uint32_t header = rd32(xecp);
        uint8_t id = (uint8_t)header;
        uint8_t next = (uint8_t)(header >> 8);
        if (id == 1u) {
            uint32_t sem = header;
            if (sem & (1u << 16)) {
                wr32(xecp, sem | (1u << 24));
                uint64_t start = psre_pit_ticks();
                bool released = false;
                for (uint64_t spins = 0; spins < XHCI_TIMEOUT_SPINS; ++spins) {
                    sem = rd32(xecp);
                    if (!(sem & (1u << 16))) { released = true; break; }
                    if (psre_pit_ticks() - start >= XHCI_TIMEOUT_TICKS) break;
                    __asm__ volatile("pause");
                }
                if (!released) { st.timeout_stage = 1; return false; }
            } else {
                wr32(xecp, sem | (1u << 24));
            }
            st.bios_handoff_done = true;
            return true;
        }
        if (!next) {
            /* No legacy-support capability is a valid platform configuration. */
            st.bios_handoff_done = true;
            return true;
        }
        uint32_t next_offset = xecp + (uint32_t)next * 4u;
        if (next_offset <= xecp || next_offset > mapped_bytes - 4u) return false;
        xecp = next_offset;
    }
    return false; /* Malformed/cyclic or excessively long capability chain. */
}
static bool halt_controller(uint32_t op) {
    uint32_t cmd = rd32(op);
    if (cmd & USBCMD_RS) wr32(op, cmd & ~USBCMD_RS);
    if (!wait_mask(op + 4u, USBSTS_HCH, USBSTS_HCH, 2)) return false;
    st.state = PSRE_M015_STATE_HALTED;
    return true;
}
static bool reset_controller(uint32_t op) {
    uint32_t cmd = rd32(op);
    if (!(rd32(op + 4u) & USBSTS_HCH)) return false;
    wr32(op, (cmd & ~USBCMD_RS) | USBCMD_HCRST);
    if (!wait_reg_clear(op, USBCMD_HCRST, 3)) return false;
    if (!wait_reg_clear(op + 4u, USBSTS_CNR, 4)) return false;
    st.state = PSRE_M015_STATE_RESET;
    return true;
}
static bool configure_controller(uint32_t op) {
    uint32_t page_size = rd32(op + 8u);
    if (!(page_size & 1u)) return false; /* Require 4 KiB page support. */

    wr32(op + 0x30u, (uint32_t)dcbaa_phys);
    wr32(op + 0x34u, (uint32_t)(dcbaa_phys >> 32));
    wr32(op + 0x18u, (uint32_t)(dma_ring_phys | 1u)); /* RCS=1 */
    wr32(op + 0x1Cu, (uint32_t)(dma_ring_phys >> 32));
    wr32(op + 0x38u, st.max_slots);

    uint32_t ir = st.runtime_offset + 0x20u; /* Interrupter 0 */
    if (ir > mapped_bytes - 0x20u) return false;
    wr32(ir + 0x04u, 0); /* IMOD: no moderation; polling, interrupts disabled. */
    wr32(ir + 0x08u, 1); /* ERSTSZ */
    wr32(ir + 0x0Cu, 0);
    wr32(ir + 0x10u, (uint32_t)(dma_ring_phys + 2u * PSRE_PAGE_SIZE));
    wr32(ir + 0x14u, 0);
    wr32(ir + 0x18u, (uint32_t)(dma_ring_phys + PSRE_PAGE_SIZE));
    wr32(ir + 0x1Cu, 0);
    /* Explicitly keep xHCI interrupt generation disabled. */
    uint32_t iman = rd32(ir);
    wr32(ir, iman & ~(1u << 1));

    uint32_t cmd = rd32(op);
    cmd &= ~USBCMD_INTE;
    wr32(op, cmd);
    __asm__ volatile("mfence" ::: "memory");
    st.state = PSRE_M015_STATE_CONFIGURED;
    return true;
}
static bool start_controller(uint32_t op) {
    uint32_t cmd = rd32(op);
    wr32(op, (cmd & ~USBCMD_INTE) | USBCMD_RS);
    st.controller_running = true; /* Treat as active until HCH proves otherwise. */
    st.safe_to_free_dma = false;
    if (!wait_mask(op + 4u, USBSTS_HCH, 0, 5)) return false;
    if (rd32(op + 4u) & USBSTS_CNR) return false;
    st.controller_running = true;
    st.safe_to_free_dma = false;
    st.state = PSRE_M015_STATE_RUNNING;
    return true;
}
static void fail_cleanup(void) {
    /* Never free DMA if bus mastering may be active or controller halt is unknown. */
    if (st.busmaster_enabled && st.controller_running) {
        uint32_t op = st.cap_length;
        (void)halt_controller(op);
        if (rd32(op + 4u) & USBSTS_HCH) {
            st.controller_running = false;
            st.safe_to_free_dma = true;
        }
    } else {
        st.safe_to_free_dma = true;
    }
    if (st.safe_to_free_dma) release_dma();
    if (mmio) {
        (void)vmm_unmap_mmio((void *)mmio, mapped_bytes);
        mmio = 0;
        st.mmio_mapped = false;
    }
    st.state = PSRE_M015_STATE_FAILED;
}
bool psre_m015_initialize(void) {
    if (st.state == PSRE_M015_STATE_RUNNING) {
        serial_write("[M015] Controller already running; refusing duplicate init.\n");
        return false;
    }
    st = (psre_m015_status_t){0};
    serial_write("\n=== PSRE M015 xHCI CONTROLLER INITIALIZATION ===\n");
    if (!find_controller()) {
        serial_write("[M015] Valid M014 live xHCI discovery is required first.\n");
        st.state = PSRE_M015_STATE_FAILED;
        return false;
    }
    st.state = PSRE_M015_STATE_DISCOVERED;
    if (!enable_pci_memory()) {
        serial_write("[M015] Cannot enable PCI Memory Space.\n");
        st.state = PSRE_M015_STATE_FAILED;
        return false;
    }
    /* Map only the registers this milestone needs. If xECP is present, map
       the bounded 64 KiB capability window so the linked capability chain
       can be walked without probing BAR sizing destructively. */
    uint64_t needed = (uint64_t)st.runtime_offset + 0x40u;
    if (needed < (uint64_t)st.doorbell_offset + ((uint64_t)st.max_slots + 1u) * 4u)
        needed = (uint64_t)st.doorbell_offset + ((uint64_t)st.max_slots + 1u) * 4u;
    if (needed < (uint64_t)st.cap_length + 0x40u)
        needed = (uint64_t)st.cap_length + 0x40u;
    const psre_m014_xhci_live_status_t *m14_map = psre_m014_get_status();
    if (m14_map && m14_map->xecp_dword_offset) needed = XHCI_MAP_BYTES;
    if (!needed || needed > XHCI_MAP_BYTES) {
        serial_write("[M015] Required register window exceeds 64 KiB safety bound.\n");
        st.state = PSRE_M015_STATE_FAILED;
        return false;
    }
    mapped_bytes = (uint32_t)((needed + PSRE_PAGE_SIZE - 1u) &
                              ~(PSRE_PAGE_SIZE - 1u));
    mmio = (volatile uint32_t *)vmm_map_mmio(st.bar_address, mapped_bytes);
    if (!mmio) {
        serial_write("[M015] xHCI BAR MMIO map failed; no controller writes.\n");
        st.state = PSRE_M015_STATE_FAILED;
        return false;
    }
    st.mmio_mapped = true;

    uint32_t caplen = rd32(0) & 0xFFu;
    uint32_t hcs1 = rd32(CAP_HCSPARAMS1);
    uint32_t hcs2 = rd32(CAP_HCSPARAMS2);
    uint32_t hcc1 = rd32(CAP_HCCPARAMS1);
    uint32_t dboff = rd32(CAP_DBOFF) & ~3u;
    uint32_t rtsoff = rd32(CAP_RTSOFF) & ~0x1Fu;
    if (caplen < 0x20u || (caplen & 3u) || !dboff || !rtsoff ||
        dboff > mapped_bytes - 4u || rtsoff > mapped_bytes - 0x40u ||
        (uint8_t)hcs1 != st.max_slots ||
        (uint8_t)(hcs1 >> 24) != st.max_ports ||
        (uint16_t)((hcs1 >> 8) & 0x7FFu) != st.max_interrupters ||
        (uint16_t)(hcc1 >> 16) !=
            psre_m014_get_status()->xecp_dword_offset) {
        serial_write("[M015] Live capability registers failed consistency/bounds checks.\n");
        fail_cleanup();
        return false;
    }
    st.cap_length = caplen;
    st.doorbell_offset = dboff;
    st.runtime_offset = rtsoff;
    st.dma_64bit_capable = (hcc1 & 1u) != 0;
    st.scratchpad_count = (uint16_t)((((hcs2 >> 27) & 0x1Fu) << 5) |
                                      ((hcs2 >> 21) & 0x1Fu));
    if (st.scratchpad_count > XHCI_MAX_SCRATCHPADS) {
        serial_write("[M015] Unsupported scratchpad count.\n");
        fail_cleanup();
        return false;
    }
    st.state = PSRE_M015_STATE_OWNERSHIP;
    if (!bios_handoff()) {
        serial_write("[M015] BIOS ownership handoff timed out; refusing reset.\n");
        fail_cleanup();
        return false;
    }

    uint32_t op = st.cap_length;
    if (!halt_controller(op)) {
        serial_write("[M015] Controller did not halt before timeout.\n");
        fail_cleanup();
        return false;
    }
    if (!reset_controller(op)) {
        serial_write("[M015] Controller reset/CNR wait failed.\n");
        fail_cleanup();
        return false;
    }
    if (!alloc_dma()) {
        serial_write("[M015] DMA allocation failed; controller remains halted.\n");
        fail_cleanup();
        return false;
    }
    if (!configure_controller(op)) {
        serial_write("[M015] Operational register/DMA configuration failed.\n");
        fail_cleanup();
        return false;
    }
    if (!enable_pci_busmaster()) {
        serial_write("[M015] PCI Bus Master Enable failed; controller remains halted.\n");
        fail_cleanup();
        return false;
    }
    if (!start_controller(op)) {
        serial_write("[M015] Controller failed to enter Running state.\n");
        fail_cleanup();
        return false;
    }
    serial_write("[M015] Controller initialized and running; interrupts disabled (polling foundation).\n");
    serial_write("[M015] USB command/event rings initialized; M017 may issue explicit commands.\n");
    psre_m015_report();
    return true;
}
const psre_m015_status_t *psre_m015_get_status(void) { return &st; }

static void hex(uint64_t v, unsigned n) {
    static const char h[] = "0123456789ABCDEF";
    for (unsigned i = 0; i < n; ++i)
        serial_write_char(h[(v >> ((n - 1u - i) * 4u)) & 15u]);
}
static void dec(uint32_t v) {
    char b[11]; unsigned n = 0;
    if (!v) { serial_write("0"); return; }
    while (v && n < sizeof(b)) { b[n++] = (char)('0' + v % 10u); v /= 10u; }
    while (n) serial_write_char(b[--n]);
}
void psre_m015_report(void) {
    serial_write("\n--- PSRE M015 xHCI INITIALIZATION STATUS ---\n");
    serial_write("[M015] State=");
    switch (st.state) {
        case PSRE_M015_STATE_DISCOVERED: serial_write("DISCOVERED"); break;
        case PSRE_M015_STATE_OWNERSHIP: serial_write("OWNERSHIP"); break;
        case PSRE_M015_STATE_HALTED: serial_write("HALTED"); break;
        case PSRE_M015_STATE_RESET: serial_write("RESET"); break;
        case PSRE_M015_STATE_CONFIGURED: serial_write("CONFIGURED"); break;
        case PSRE_M015_STATE_RUNNING: serial_write("RUNNING"); break;
        case PSRE_M015_STATE_FAILED: serial_write("FAILED"); break;
        default: serial_write("IDLE"); break;
    }
    serial_write("\n");
    if (st.controller_found) {
        serial_write("[M015] PCI "); hex(st.bus,2); serial_write(":");
        hex(st.device,2); serial_write("."); hex(st.function,1);
        serial_write(" VID:DID="); hex(st.vendor_id,4); serial_write(":");
        hex(st.device_id,4); serial_write("\n[M015] BAR=0x");
        hex(st.bar_address,16); serial_write("\n");
    }
    serial_write("[M015] BIOS handoff="); serial_write(st.bios_handoff_done?"yes":"no");
    serial_write(" DMA="); serial_write(st.dma_ready?"ready":"no");
    serial_write(" BusMaster="); serial_write(st.busmaster_enabled?"enabled":"disabled");
    serial_write(" Running="); serial_write(st.controller_running?"yes":"no");
    serial_write("\n[M015] slots="); dec(st.max_slots);
    serial_write(" ports="); dec(st.max_ports);
    serial_write(" interrupters="); dec(st.max_interrupters);
    serial_write(" scratchpads="); dec(st.scratchpad_count);
    serial_write("\n[M015] timeout stage="); dec(st.timeout_stage);
    serial_write("\n[M015] DMA release safety="); serial_write(st.safe_to_free_dma?"safe":"not established");
    serial_write("\n");
    serial_write("[M015] USB enumeration and Mass Storage are not implemented.\n");
}


bool psre_m015_submit_command(const psre_xhci_trb_t *command) {
    if (!command || st.state != PSRE_M015_STATE_RUNNING || !mmio ||
        !command_ring || command_enqueue >= XHCI_RING_TRBS - 1u) return false;
    xhci_trb_t *dst = &command_ring[command_enqueue];
    dst->parameter = command->parameter;
    dst->status = command->status;
    dst->control = (command->control & ~1u) | command_cycle;
    __asm__ volatile("mfence" ::: "memory");
    ++command_enqueue;
    if (command_enqueue == XHCI_RING_TRBS - 1u) {
        uint32_t link = (XHCI_TRB_TYPE_LINK << XHCI_TRB_TYPE_SHIFT) | (1u << 1);
        command_ring[XHCI_RING_TRBS - 1u].control = link | command_cycle;
        __asm__ volatile("mfence" ::: "memory");
        command_enqueue = 0;
        command_cycle ^= 1u;
    }
    /* Doorbell 0 rings the command ring. */
    volatile uint32_t *doorbell = (volatile uint32_t *)((uintptr_t)mmio + st.doorbell_offset);
    doorbell[0] = 0;
    __asm__ volatile("mfence" ::: "memory");
    return true;
}

bool psre_m015_wait_event(psre_xhci_trb_t *event, uint32_t timeout_ticks) {
    if (!event || st.state != PSRE_M015_STATE_RUNNING || !mmio || !event_ring)
        return false;
    uint32_t ir = st.runtime_offset + 0x20u;
    uint64_t start = psre_pit_ticks();
    for (uint64_t spins = 0; spins < XHCI_TIMEOUT_SPINS; ++spins) {
        xhci_trb_t *src = &event_ring[event_dequeue];
        __asm__ volatile("lfence" ::: "memory");
        if ((src->control & 1u) == event_cycle) {
            event->parameter = src->parameter;
            event->status = src->status;
            event->control = src->control;
            ++event_dequeue;
            if (event_dequeue == XHCI_RING_TRBS) {
                event_dequeue = 0;
                event_cycle ^= 1u;
            }
            uint64_t erdp = dma_ring_phys + PSRE_PAGE_SIZE +
                            (uint64_t)event_dequeue * sizeof(xhci_trb_t);
            wr32(ir + 0x18u, (uint32_t)erdp | (1u << 3)); /* EHB: acknowledge event dequeue. */
            wr32(ir + 0x1Cu, (uint32_t)(erdp >> 32));
            return true;
        }
        if (psre_pit_ticks() - start >= timeout_ticks) break;
        __asm__ volatile("pause");
    }
    return false;
}


bool psre_m015_wait_command_event(psre_xhci_trb_t *event, uint32_t timeout_ticks) {
    return psre_m015_wait_event(event, timeout_ticks);
}

bool psre_m015_dma_alloc_pages(uint32_t pages, uint64_t *physical, void **virtual_address) {
    uint8_t *virt = 0;
    if (!pages || !physical || !virtual_address ||
        st.state != PSRE_M015_STATE_RUNNING || !st.dma_ready) return false;
    if (!alloc_dma_block(pages, physical, &virt)) return false;
    *virtual_address = virt;
    /* DMA pages remain pinned while xHCI may own them. */
    return true;
}

bool psre_m015_ring_doorbell(uint8_t slot_id, uint8_t target) {
    if (st.state != PSRE_M015_STATE_RUNNING || !mmio || slot_id > st.max_slots) return false;
    uint64_t offset = (uint64_t)st.doorbell_offset + (uint64_t)slot_id * 4u;
    if (offset > mapped_bytes - 4u) return false;
    volatile uint32_t *doorbell = (volatile uint32_t *)((uintptr_t)mmio + (uintptr_t)offset);
    *doorbell = (uint32_t)target;
    __asm__ volatile("mfence" ::: "memory");
    return true;
}

uint8_t psre_m015_context_size(void) {
    if (st.state != PSRE_M015_STATE_RUNNING || !mmio) return 0;
    return (rd32(CAP_HCCPARAMS1) & (1u << 2)) ? 64u : 32u;
}


bool psre_m015_set_dcbaa_entry(uint8_t slot_id, uint64_t device_context_physical) {
    if (st.state != PSRE_M015_STATE_RUNNING || !dcbaa || slot_id == 0 ||
        slot_id > st.max_slots || (device_context_physical & 0x3Fu)) return false;
    dcbaa[slot_id] = device_context_physical;
    __asm__ volatile("mfence" ::: "memory");
    return true;
}
