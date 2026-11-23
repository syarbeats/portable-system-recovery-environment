#include <psre/m055/input_service.h>
#include <psre/m049/usb_gui.h>
#include <psre/m025/input.h>
#include <psre/serial.h>
static psre_m055_status_t st;
bool psre_m055_initialize(void) {
    st=(psre_m055_status_t){0};
    st.initialized=true;
    const psre_m049_status_t *usb=psre_m049_get_status();
    const psre_m025_status_t *input=psre_m025_get_status();
    st.usb_input_available=usb && usb->initialized && usb->gui_ready;
    st.keyboard_active=input && input->keyboard_active;
    st.mouse_active=input && input->mouse_active;
    serial_write("[M055] HID input service initialized; desktop boot is independent.\\n");
    serial_write(st.usb_input_available?
        "[M055] USB input source available; polling enabled.\\n":
        "[M055] No active USB input source; desktop remains available.\\n");
    return true;
}
bool psre_m055_poll(uint32_t timeout_ticks) {
    if(!st.initialized || !timeout_ticks) return false;
    ++st.poll_cycles;
    const psre_m049_status_t *usb=psre_m049_get_status();
    const psre_m025_status_t *input=psre_m025_get_status();
    st.usb_input_available=usb && usb->initialized && usb->gui_ready;
    st.keyboard_active=input && input->keyboard_active;
    st.mouse_active=input && input->mouse_active;
    if(!st.usb_input_available) { ++st.idle_cycles; return false; }
    bool activity=psre_m049_poll(timeout_ticks);
    ++st.active_poll_cycles;
    if(!activity && input && input->transfer_errors) ++st.poll_errors;
    return activity;
}
const psre_m055_status_t *psre_m055_get_status(void) { return &st; }
static void dec(uint64_t v) {
    char b[20]; uint8_t n=0;
    if(!v){serial_write("0");return;}
    while(v && n<sizeof(b)){b[n++]=(char)('0'+v%10u);v/=10u;}
    while(n)serial_write_char(b[--n]);
}
void psre_m055_report(void) {
    serial_write("\\n--- M055 USB HID INPUT SERVICE ---\\nInitialized=");
    serial_write(st.initialized?"yes":"no");
    serial_write(" USB=");serial_write(st.usb_input_available?"ready":"unavailable");
    serial_write(" keyboard=");serial_write(st.keyboard_active?"active":"inactive");
    serial_write(" mouse=");serial_write(st.mouse_active?"active":"inactive");
    serial_write(" polls=");dec(st.poll_cycles);
    serial_write(" active_polls=");dec(st.active_poll_cycles);
    serial_write(" idle=");dec(st.idle_cycles);
    serial_write(" errors=");dec(st.poll_errors);serial_write("\\n");
}
