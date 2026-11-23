#include <psre/m058/diagnostics.h>
#include <psre/m015/xhci_init.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/m021/hid.h>
#include <psre/m022/interrupt.h>
#include <psre/m025/input.h>
#include <psre/m030/input.h>
#include <psre/serial.h>

static void dec(uint32_t v) {
    char b[10]; unsigned n=0;
    if (!v) { serial_write("0"); return; }
    while (v && n < sizeof b) { b[n++]=(char)('0'+v%10u); v/=10u; }
    while (n) serial_write_char(b[--n]);
}
static void flag(const char *s, bool v) { serial_write(s); serial_write(v?"yes":"no"); }

void psre_m058_initialize(void) {
    serial_write("\n=== PSRE M058 XHCI ENDPOINT AUDIT ===\n");
    psre_m058_report();
}

void psre_m058_report(void) {
    const psre_m015_status_t *hc=psre_m015_get_status();
    const psre_m018_status_t *dev=psre_m018_get_status();
    const psre_m021_status_t *hid=psre_m021_get_status();
    const psre_m025_status_t *in=psre_m025_get_status();
    const psre_m030_status_t *rt=psre_m030_get_status();
    const psre_m022_status_t *all=psre_m022_get_status();
    flag("[M058] xHCI running=",hc&&hc->state==PSRE_M015_STATE_RUNNING); serial_write("\n");
    flag("[M058] USB address assigned=",dev&&dev->address_assigned); serial_write("\n");
    if (dev) { serial_write("[M058] slot=");dec(dev->slot_id);serial_write(" speed-id=");dec(dev->speed_id);serial_write(" VID=");dec(dev->vendor_id);serial_write(" PID=");dec(dev->product_id);serial_write("\n"); }
    flag("[M058] HID descriptor parse=",hid&&hid->success); serial_write("\n");
    if (hid) { serial_write("[M058] interfaces=");dec(hid->hid_interface_count);serial_write(" keyboards=");dec(hid->keyboard_count);serial_write(" mice=");dec(hid->mouse_count);serial_write("\n"); }
    if (in) { flag("[M058] keyboard-active=",in->keyboard_active);serial_write(" ");flag("mouse-active=",in->mouse_active);serial_write("\n"); }
    if (all) { serial_write("[M058] transfers attempted=");dec(all->transfer_attempts);serial_write(" completed=");dec(all->transfer_count);serial_write(" timeout=");dec(all->timeout_count);serial_write(" completion-errors=");dec(all->completion_errors);serial_write(" unmatched-events=");dec(all->unmatched_events);serial_write("\n"); }
    if (rt) { serial_write("[M058] router received=");dec(rt->events_received);serial_write(" dispatched=");dec(rt->events_dispatched);serial_write(" dropped=");dec(rt->events_dropped);serial_write("\n"); }
    serial_write("[M058] Diagnostic-only audit; no PCI writes, xHCI reset, USB re-enumeration, or Desktop boot-path changes.\n");
}
