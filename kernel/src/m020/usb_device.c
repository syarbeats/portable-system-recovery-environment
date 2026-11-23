#include <psre/m020/usb_device.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/m019/usb_config.h>
#include <psre/serial.h>
#include <stdint.h>
#include <stdbool.h>

static psre_m020_status_t st;
static psre_usb_device_t registry[PSRE_M020_REGISTRY_CAPACITY];

static void dec(uint32_t v) {
    char b[11]; unsigned n=0;
    if(!v){serial_write("0");return;}
    while(v && n<sizeof(b)){b[n++]=(char)('0'+v%10u);v/=10u;}
    while(n)serial_write_char(b[--n]);
}
static psre_usb_class_kind_t classify(uint8_t cls, uint8_t sub, uint8_t proto) {
    (void)sub; (void)proto;
    switch(cls) {
        case 0x03u: return PSRE_USB_CLASS_HID;
        case 0x08u: return PSRE_USB_CLASS_MASS_STORAGE;
        case 0x09u: return PSRE_USB_CLASS_HUB;
        case 0x02u: case 0x0Au: return PSRE_USB_CLASS_CDC;
        case 0xFFu: return PSRE_USB_CLASS_VENDOR;
        case 0x00u: return PSRE_USB_CLASS_UNKNOWN;
        default: return PSRE_USB_CLASS_OTHER;
    }
}
static void count_class(psre_usb_class_kind_t k) {
    ++st.interfaces_classified;
    switch(k) {
        case PSRE_USB_CLASS_HID: ++st.hid_interfaces; break;
        case PSRE_USB_CLASS_MASS_STORAGE: ++st.mass_storage_interfaces; break;
        case PSRE_USB_CLASS_HUB: ++st.hub_interfaces; break;
        case PSRE_USB_CLASS_CDC: ++st.cdc_interfaces; break;
        case PSRE_USB_CLASS_VENDOR: ++st.vendor_interfaces; break;
        case PSRE_USB_CLASS_OTHER: ++st.other_interfaces; break;
        default: break;
    }
}
bool psre_m020_activate_configuration(void) {
    st=(psre_m020_status_t){0};  /* per-device activation; registry is preserved */
    st.attempted=true;
    const psre_m018_status_t *m18=psre_m018_get_status();
    const psre_m019_status_t *m19=psre_m019_get_status();
    if(!m18 || !m18->descriptor_read || !m19 || !m19->success ||
       !m19->configuration_value) {
        serial_write("[M020] Successful M018 and M019 are required first.\n");
        st.failure_stage=1; return false;
    }
    /* Standard device request: SET_CONFIGURATION, no data stage. */
    if(!psre_m018_control_no_data(0x00u,9u,m19->configuration_value,0u)) {
        serial_write("[M020] SET_CONFIGURATION control transfer failed.\n");
        st.failure_stage=2; return false;
    }
    st.configured=true;
    st.selected_configuration=m19->configuration_value;
    psre_usb_device_t *dev=&registry[0];
    *dev=(psre_usb_device_t){0};
    dev->present=true; dev->configured=true;
    dev->slot_id=m18->slot_id; dev->port=m18->port; dev->speed_id=m18->speed_id;
    dev->configuration_value=m19->configuration_value;
    dev->vendor_id=m18->vendor_id; dev->product_id=m18->product_id;
    dev->interface_count=m19->interface_count;
    dev->primary_class=PSRE_USB_CLASS_UNKNOWN;
    for(uint8_t i=0;i<m19->interface_count;++i) {
        /* Class identification is interface-based for composite devices. */
        psre_usb_class_kind_t k=classify(m19->interfaces[i].class_code,
                                        m19->interfaces[i].subclass,
                                        m19->interfaces[i].protocol);
        count_class(k);
        if(i==0u) dev->primary_class=k;
    }
    serial_write("[M020] Configuration ");dec(st.selected_configuration);
    serial_write(" activated; interface classes discovered=");dec(st.interfaces_classified);
    serial_write(".\n");
    return true;
}
bool psre_m020_set_alternate(uint8_t interface_number, uint8_t alternate_setting) {
    if(!st.configured) {
        serial_write("[M020] Activate configuration first with m020-configure.\n");
        return false;
    }
    const psre_m019_status_t *m19=psre_m019_get_status();
    bool found=false;
    for(uint8_t i=0;i<m19->interface_count;++i) {
        if(m19->interfaces[i].number==interface_number &&
           m19->interfaces[i].alternate_setting==alternate_setting) {
            found=true; break;
        }
    }
    if(!found) {
        serial_write("[M020] Requested interface/alternate setting is not in parsed descriptors.\n");
        st.failure_stage=3; return false;
    }
    /* Standard interface request: SET_INTERFACE. */
    if(!psre_m018_control_no_data(0x01u,11u,alternate_setting,interface_number)) {
        serial_write("[M020] SET_INTERFACE control transfer failed.\n");
        st.failure_stage=4; return false;
    }
    st.last_interface=interface_number;
    st.last_alternate=alternate_setting;
    serial_write("[M020] Interface ");dec(interface_number);
    serial_write(" alternate setting ");dec(alternate_setting);
    serial_write(" selected.\n");
    return true;
}
static const char *class_name(psre_usb_class_kind_t k) {
    switch(k) {
        case PSRE_USB_CLASS_HID:return "HID";
        case PSRE_USB_CLASS_MASS_STORAGE:return "MASS_STORAGE";
        case PSRE_USB_CLASS_HUB:return "HUB";
        case PSRE_USB_CLASS_CDC:return "CDC";
        case PSRE_USB_CLASS_VENDOR:return "VENDOR";
        case PSRE_USB_CLASS_OTHER:return "OTHER";
        default:return "UNKNOWN";
    }
}
void psre_m020_report(void) {
    serial_write("\n--- PSRE M020 USB DEVICE REGISTRY ---\n");
    serial_write("Attempted=");serial_write(st.attempted?"yes":"no");
    serial_write(" Configured=");serial_write(st.configured?"yes":"no");
    serial_write(" Configuration=");dec(st.selected_configuration);
    serial_write(" Interfaces classified=");dec(st.interfaces_classified);
    serial_write("\nClass counts: HID=");dec(st.hid_interfaces);
    serial_write(" MassStorage=");dec(st.mass_storage_interfaces);
    serial_write(" Hub=");dec(st.hub_interfaces);
    serial_write(" CDC=");dec(st.cdc_interfaces);
    serial_write(" Vendor=");dec(st.vendor_interfaces);
    serial_write(" Other=");dec(st.other_interfaces);
    serial_write(" Failure stage=");dec(st.failure_stage);serial_write("\n");
    for(uint8_t i=0;i<PSRE_M020_REGISTRY_CAPACITY;++i) {
        const psre_usb_device_t *d=&registry[i];
        if(!d->present) continue;
        serial_write("Device slot=");dec(d->slot_id);serial_write(" port=");dec(d->port);
        serial_write(" VID=0x");
        static const char h[]="0123456789ABCDEF";
        for(int b=12;b>=0;b-=4)serial_write_char(h[(d->vendor_id>>b)&15u]);
        serial_write(" PID=0x");
        for(int b=12;b>=0;b-=4)serial_write_char(h[(d->product_id>>b)&15u]);
        serial_write(" class=");serial_write(class_name(d->primary_class));
        serial_write(" configured=");serial_write(d->configured?"yes":"no");
        serial_write("\n");
    }
    serial_write("[M020] Class discovery only; HID and Mass Storage class drivers are not implemented.\n");
}
const psre_m020_status_t *psre_m020_get_status(void){return &st;}
const psre_usb_device_t *psre_m020_registry_get(uint8_t index) {
    return index<PSRE_M020_REGISTRY_CAPACITY?&registry[index]:0;
}
