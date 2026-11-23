#include <psre/m010/usb.h>
#include <psre/m004/pci.h>
#include <psre/serial.h>
#include <stdint.h>

static psre_usb_stats_t stats;
static const char *kind(uint8_t p) {
    switch (p) {
        case 0x00: return "UHCI";
        case 0x10: return "OHCI";
        case 0x20: return "EHCI";
        case 0x30: return "xHCI";
        default: return "Unknown USB HC";
    }
}
const char *psre_m010_usb_hc_name(uint8_t p) { return kind(p); }
bool psre_m010_usb_initialize(void) {
    const psre_pci_stats_t *pci = psre_pci_get_stats();
    const psre_pci_device_t *dev = psre_pci_get_devices();
    stats = (psre_usb_stats_t){0};
    if (!pci || !pci->config_mechanism_1_available) return false;
    stats.pci_inventory_available = true;
    for (uint32_t i=0; i<pci->stored; ++i) {
        if (dev[i].class_code != 0x0C || dev[i].subclass != 0x03) continue;
        ++stats.controllers;
        switch (dev[i].prog_if) {
            case 0x00: ++stats.uhci; break;
            case 0x10: ++stats.ohci; break;
            case 0x20: ++stats.ehci; break;
            case 0x30: ++stats.xhci; break;
            default: ++stats.unknown; break;
        }
    }
    return true;
}
const psre_usb_stats_t *psre_m010_usb_get_stats(void) { return &stats; }
static void hex(uint32_t v, unsigned n) {
    static const char h[]="0123456789ABCDEF";
    for (unsigned i=0;i<n;i++) serial_write_char(h[(v >> ((n-1-i)*4)) & 15]);
}
void psre_m010_usb_report(void) {
    const psre_pci_stats_t *pci=psre_pci_get_stats();
    const psre_pci_device_t *dev=psre_pci_get_devices();
    serial_write("\n--- PSRE USB HOST CONTROLLER INVENTORY (M010) ---\n");
    if (!stats.pci_inventory_available) {
        serial_write("[M010] PCI inventory unavailable; run hardware discovery first.\n");
        return;
    }
    uint32_t found=0;
    for (uint32_t i=0;i<pci->stored;i++) {
        const psre_pci_device_t *d=&dev[i];
        if(d->class_code!=0x0C || d->subclass!=0x03) continue;
        ++found;
        serial_write("[USB HC] PCI ");hex(d->bus,2);serial_write(":");hex(d->device,2);
        serial_write(".");hex(d->function,1);serial_write(" VID:DID=");
        hex(d->vendor_id,4);serial_write(":");hex(d->device_id,4);
        serial_write(" type=");serial_write(kind(d->prog_if));
        serial_write(" command=0x");hex(d->command,4);serial_write("\n");
        for(uint8_t b=0;b<d->bar_count;b++){
            const psre_pci_bar_t *bar=&d->bars[b];
            serial_write("  BAR");hex(bar->index,1);
            serial_write(bar->kind==1?" I/O ":" MMIO ");
            serial_write("0x");
            for(int s=15;s>=0;s--)serial_write_char("0123456789ABCDEF"[(bar->address>>(s*4))&15]);
            if(bar->is_64bit)serial_write(" 64-bit");
            serial_write("\n");
        }
    }
    if(!found)serial_write("[M010] No PCI USB host controller found.\n");
    serial_write("[M010] Total=");hex(stats.controllers,4);
    serial_write(" UHCI=");hex(stats.uhci,4);serial_write(" OHCI=");hex(stats.ohci,4);
    serial_write(" EHCI=");hex(stats.ehci,4);serial_write(" xHCI=");hex(stats.xhci,4);
    serial_write(" Unknown=");hex(stats.unknown,4);serial_write("\n");
    serial_write("[M010] Discovery only: controller MMIO is not modified.\n");
}
