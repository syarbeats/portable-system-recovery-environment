#include <psre/m024/mouse.h>
#include <psre/m021/hid.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/serial.h>
static psre_m024_status_t st;
static void dec(uint32_t v){char b[10];unsigned n=0;if(!v){serial_write("0");return;}while(v&&n<sizeof(b)){b[n++]=(char)('0'+v%10u);v/=10u;}while(n)serial_write_char(b[--n]);}
bool psre_m024_initialize(void){
    st=(psre_m024_status_t){0};
    const psre_m021_status_t *h=psre_m021_get_status();
    const psre_m018_status_t *d=psre_m018_get_status();
    if(!h||!h->success||!d||!d->address_assigned)return false;
    for(uint8_t i=0;i<h->hid_interface_count;++i){
        const psre_m021_hid_interface_t *it=&h->interfaces[i];
        if(it->present&&(it->kind==PSRE_HID_DEVICE_MOUSE||it->kind==PSRE_HID_DEVICE_KEYBOARD_MOUSE)&&it->protocol==2u&&it->subclass==1u&&it->interrupt_in_endpoint){
            if(!psre_m018_control_no_data(0x21u,0x0Bu,0u,it->interface_number)){
                serial_write("[M024] SET_PROTOCOL(Boot) failed.\n");return false;
            }
            st.initialized=true;st.interface_number=it->interface_number;
            st.endpoint_address=it->interrupt_in_endpoint;st.max_packet_size=it->max_packet_size;st.interval=it->interval;
            serial_write("[M024] Boot mouse interface selected.\n");return true;
        }
    }
    serial_write("[M024] No HID boot mouse interface found.\n");return false;
}
bool psre_m024_process_report(const uint8_t *r,uint16_t n){
    if(!st.initialized||!r||n<3u){++st.malformed_reports;return false;}
    st.buttons=(uint8_t)(r[0]&0x07u);
    st.delta_x=(int8_t)r[1];st.delta_y=(int8_t)r[2];
    st.wheel=0;
    ++st.reports;return true;
}
const psre_m024_status_t *psre_m024_get_status(void){return &st;}
void psre_m024_report(void){
    serial_write("\n--- M024 USB HID MOUSE ---\nInitialized=");
    serial_write(st.initialized?"yes":"no");serial_write(" IF=");
    dec(st.interface_number);serial_write(" EP=0x");
    static const char x[]="0123456789ABCDEF";
    serial_write_char(x[st.endpoint_address>>4]);serial_write_char(x[st.endpoint_address&15u]);
    serial_write(" buttons=");dec(st.buttons);serial_write(" dx=");dec((uint16_t)st.delta_x);
    serial_write(" dy=");dec((uint16_t)st.delta_y);serial_write(" reports=");
    dec(st.reports);serial_write(" malformed=");dec(st.malformed_reports);serial_write("\n");
}
