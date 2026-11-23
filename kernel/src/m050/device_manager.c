#include <psre/m050/device_manager.h>
#include <psre/m049/usb_gui.h>
#include <psre/serial.h>
static psre_m050_device_status_t state;
bool psre_m050_device_manager_initialize(void) {
    state=(psre_m050_device_status_t){0}; state.initialized=true;
    psre_m050_device_manager_refresh();
    serial_write("[M050] Device manager initialized; monitoring enumerated HID state.\n");
    return true;
}
void psre_m050_device_manager_refresh(void) {
    if(!state.initialized) return;
    ++state.scans;
    const psre_m049_status_t *s=psre_m049_get_status();
    bool keyboard=s && s->initialized && s->keyboard_ready;
    bool mouse=s && s->initialized && s->mouse_ready;
    if(keyboard!=state.keyboard_present || mouse!=state.mouse_present) ++state.transitions;
    state.keyboard_present=keyboard; state.mouse_present=mouse;
    state.usb_stack_available=s && s->controller_ready;
}
const psre_m050_device_status_t *psre_m050_device_manager_get_status(void){return &state;}
void psre_m050_device_manager_report(void){
    serial_write("[M050] USB stack=");serial_write(state.usb_stack_available?"ready":"not-ready");
    serial_write(" keyboard=");serial_write(state.keyboard_present?"present":"absent");
    serial_write(" mouse=");serial_write(state.mouse_present?"present":"absent");
    serial_write(" scans=");
    /* Counters are intentionally reported as raw state via status API. */
    serial_write("(see status API)\n");
}
