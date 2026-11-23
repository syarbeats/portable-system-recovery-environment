#include <psre/m056/diagnostics.h>
#include <psre/m031/framebuffer.h>
#include <psre/m021/hid.h>
#include <psre/m022/interrupt.h>
#include <psre/m025/input.h>
#include <psre/m030/input.h>
#include <psre/m049/usb_gui.h>
#include <psre/serial.h>
static psre_m056_status_t st;
static void dec(uint32_t v){char b[10];uint8_t n=0;if(!v){serial_write("0");return;}while(v&&n<sizeof b){b[n++]=(char)('0'+v%10u);v/=10u;}while(n)serial_write_char(b[--n]);}
static void flag(const char *n,bool v){serial_write(n);serial_write(v?"yes":"no");}
bool psre_m056_initialize(void){
 st=(psre_m056_status_t){0};
 const psre_m031_framebuffer_t *fb=psre_m031_get();
 const psre_m021_status_t *hid=psre_m021_get_status();
 const psre_m049_status_t *usb=psre_m049_get_status();
 const psre_m025_status_t *in=psre_m025_get_status();
 const psre_m030_status_t *rt=psre_m030_get_status();
 st.framebuffer_ready=fb&&fb->available;st.usb_controller_ready=usb&&usb->controller_ready;
 st.device_enumerated=usb&&usb->device_enumerated;st.desktop_ready=usb&&usb->gui_ready;
 st.hid_probe_ok=hid&&hid->success;
 if(hid){st.hid_interfaces=hid->hid_interface_count;st.hid_keyboard_interfaces=hid->keyboard_count;st.hid_mouse_interfaces=hid->mouse_count;}
 st.keyboard_active=in&&in->keyboard_active;st.mouse_active=in&&in->mouse_active;
 if(in){st.keyboard_polls=in->keyboard_polls;st.mouse_polls=in->mouse_polls;st.input_transfer_errors=in->transfer_errors;}
 st.input_router_ready=rt&&rt->initialized;if(rt){st.routed_events=rt->events_received;st.dropped_events=rt->events_dropped;}
 const psre_m022_status_t *x=psre_m022_get_status();
 if(x){st.transfer_attempts=x->transfer_attempts;st.transfer_successes=x->transfer_count;st.transfer_timeouts=x->timeout_count;st.transfer_completion_errors=x->completion_errors;st.transfer_doorbell_errors=x->doorbell_failures;st.transfer_unmatched_events=x->unmatched_events;}
 st.initialized=true;
 serial_write("\n=== PSRE M056 USB HID DIAGNOSTICS ===\n");
 flag("[M056] Framebuffer=",st.framebuffer_ready);serial_write("\n");
 flag("[M056] xHCI controller=",st.usb_controller_ready);serial_write("\n");
 flag("[M056] USB device enumerated=",st.device_enumerated);serial_write("\n");
 flag("[M056] HID descriptor probe=",st.hid_probe_ok);serial_write("\n");
 serial_write("[M056] HID interfaces=");dec(st.hid_interfaces);serial_write(" keyboard=");dec(st.hid_keyboard_interfaces);serial_write(" mouse=");dec(st.hid_mouse_interfaces);serial_write("\n");
 flag("[M056] Keyboard endpoint active=",st.keyboard_active);serial_write("\n");
 flag("[M056] Mouse endpoint active=",st.mouse_active);serial_write("\n");
 flag("[M056] M030 input router=",st.input_router_ready);serial_write("\n");
 flag("[M056] Desktop=",st.desktop_ready);serial_write("\n");
 serial_write("[M056] Transfer attempts=");dec(st.transfer_attempts);serial_write(" success=");dec(st.transfer_successes);serial_write(" timeout=");dec(st.transfer_timeouts);serial_write(" completion-errors=");dec(st.transfer_completion_errors);serial_write(" doorbell-errors=");dec(st.transfer_doorbell_errors);serial_write(" unmatched-events=");dec(st.transfer_unmatched_events);serial_write("\n");
 serial_write("[M056] Input polls keyboard=");dec(st.keyboard_polls);serial_write(" mouse=");dec(st.mouse_polls);serial_write(" transfer-errors=");dec(st.input_transfer_errors);serial_write(" routed-events=");dec(st.routed_events);serial_write(" dropped-events=");dec(st.dropped_events);serial_write("\n");
 serial_write("[M056] Read-only snapshot; no USB reset/re-enumeration performed.\n");
 return true;
}
void psre_m056_report(void){if(!st.initialized)serial_write("[M056] Diagnostics not initialized.\n");else serial_write("[M056] Boot snapshot captured; initialize again to refresh counters.\n");}
const psre_m056_status_t *psre_m056_get_status(void){return &st;}
