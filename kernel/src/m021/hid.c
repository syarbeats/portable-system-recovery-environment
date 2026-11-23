#include <psre/m021/hid.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/m019/usb_config.h>
#include <psre/m020/usb_device.h>
#include <psre/serial.h>
#include <stdint.h>
#include <stdbool.h>

#define M021_HID_CLASS 0x03u
#define M021_DESC_REPORT 0x22u
#define M021_MAX_REPORT_DESCRIPTOR 512u
#define M021_ENDPOINT_INTERRUPT 0x03u
#define M021_ENDPOINT_IN 0x80u

static psre_m021_status_t st;
static uint8_t report_buffer[M021_MAX_REPORT_DESCRIPTOR];

static void dec(uint32_t v) {
    char b[11]; unsigned n=0;
    if(!v){serial_write("0");return;}
    while(v && n<sizeof(b)){b[n++]=(char)('0'+v%10u);v/=10u;}
    while(n)serial_write_char(b[--n]);
}
static void hex8(uint8_t v) {
    static const char h[]="0123456789ABCDEF";
    serial_write_char(h[v>>4]);serial_write_char(h[v&15u]);
}
static bool get_report_descriptor(uint8_t interface_number,uint16_t length) {
    return psre_m018_control_in(0x81u,6u,
        (uint16_t)((M021_DESC_REPORT<<8)|0u),interface_number,
        length,report_buffer,sizeof(report_buffer));
}
static int find_report_id(psre_m021_hid_interface_t *it,uint8_t id) {
    for(uint8_t i=0;i<it->report_id_count;++i)
        if(it->report_ids[i].report_id==id) return (int)i;
    if(it->report_id_count>=PSRE_M021_MAX_REPORT_IDS) return -1;
    uint8_t n=it->report_id_count++;
    it->report_ids[n]=(psre_m021_report_id_t){0};
    it->report_ids[n].report_id=id;
    return (int)n;
}

/* Bounded HID short/long item parser. It accounts Input report bits and
   recognizes Generic Desktop Mouse and Keyboard usages. */
static bool parse_report_descriptor(psre_m021_hid_interface_t *it,
                                    const uint8_t *data,uint16_t length) {
    uint32_t usage_page=0, report_size=0, report_count=0;
    uint32_t local_usage=0;
    uint8_t report_id=0;
    bool app_keyboard=false, app_mouse=false;
    uint16_t pos=0;
    while(pos<length) {
        uint8_t prefix=data[pos++];
        if(prefix==0xFEu) {
            if((uint16_t)(length-pos)<2u) return false;
            uint8_t n=data[pos++];
            ++pos; /* long item tag */
            if(n>length-pos) return false;
            pos=(uint16_t)(pos+n);
            continue;
        }
        uint8_t size_code=(uint8_t)(prefix&3u);
        uint8_t size=(size_code==3u)?4u:size_code;
        uint8_t type=(uint8_t)((prefix>>2)&3u);
        uint8_t tag=(uint8_t)(prefix>>4);
        if(size>length-pos) return false;
        uint32_t value=0;
        for(uint8_t i=0;i<size;++i) value|=(uint32_t)data[pos+i]<<(8u*i);
        pos=(uint16_t)(pos+size);
        if(type==1u) { /* Global */
            if(tag==0u) usage_page=value;
            else if(tag==7u) report_size=value;
            else if(tag==8u) {
                if(value==0u || value>255u) return false;
                report_id=(uint8_t)value;
            } else if(tag==9u) report_count=value;
        } else if(type==2u) { /* Local */
            if(tag==0u) local_usage=value;
        } else if(type==0u) { /* Main */
            if(tag==8u) { /* Input */
                if(report_size>2048u || report_count>2048u ||
                   (report_size && report_count>65535u/report_size))
                    return false;
                uint32_t bits=report_size*report_count;
                int idx=find_report_id(it,report_id);
                if(idx<0 || bits>65535u-it->report_ids[idx].input_bits)
                    return false;
                psre_m021_report_id_t *rid=&it->report_ids[idx];
                rid->input_bits=(uint16_t)(rid->input_bits+bits);
                if(app_keyboard || usage_page==0x07u) rid->has_keyboard=true;
                if(app_mouse || (usage_page==0x01u && local_usage==0x02u))
                    rid->has_mouse=true;
                local_usage=0;
            } else if(tag==9u || tag==11u) { /* Output / Feature */
                local_usage=0;
            } else if(tag==10u) { /* Collection */
                if(usage_page==0x01u && local_usage==0x02u) app_mouse=true;
                if(usage_page==0x07u && local_usage==0x06u) app_keyboard=true;
                local_usage=0;
            } else if(tag==12u) { /* End Collection */
                local_usage=0;
            }
        }
    }
    for(uint8_t i=0;i<it->report_id_count;++i) {
        psre_m021_report_id_t *rid=&it->report_ids[i];
        uint32_t bits=rid->input_bits+(rid->report_id?8u:0u);
        rid->input_bytes=(uint16_t)((bits+7u)/8u);
    }
    return it->report_id_count!=0u;
}
static psre_hid_device_kind_t determine_kind(const psre_m021_hid_interface_t *it) {
    bool keyboard=false,mouse=false;
    for(uint8_t i=0;i<it->report_id_count;++i) {
        keyboard|=it->report_ids[i].has_keyboard;
        mouse|=it->report_ids[i].has_mouse;
    }
    if(keyboard && mouse) return PSRE_HID_DEVICE_KEYBOARD_MOUSE;
    if(keyboard) return PSRE_HID_DEVICE_KEYBOARD;
    if(mouse) return PSRE_HID_DEVICE_MOUSE;
    return PSRE_HID_DEVICE_OTHER;
}
bool psre_m021_probe_hid(void) {
    st=(psre_m021_status_t){0};
    st.attempted=true;
    const psre_m019_status_t *m19=psre_m019_get_status();
    const psre_m020_status_t *m20=psre_m020_get_status();
    if(!m19 || !m19->success || !m20 || !m20->configured) {
        serial_write("[M021] Successful M019 parsing and M020 configuration activation are required.\n");
        st.failure_stage=1;return false;
    }
    for(uint8_t i=0;i<m19->interface_count;++i) {
        const psre_m019_interface_t *src=&m19->interfaces[i];
        if(src->class_code!=M021_HID_CLASS || src->alternate_setting!=0u)
            continue;
        if(st.hid_interface_count>=PSRE_M021_MAX_HID_INTERFACES) {
            st.failure_stage=2;return false;
        }
        psre_m021_hid_interface_t *dst=&st.interfaces[st.hid_interface_count++];
        dst->present=true;
        dst->interface_number=src->number;
        dst->subclass=src->subclass;
        dst->protocol=src->protocol;
        dst->report_descriptor_length=src->hid_report_descriptor_length;
        if(!src->hid_descriptor_present || !src->hid_report_descriptor_length ||
           src->hid_report_descriptor_length>M021_MAX_REPORT_DESCRIPTOR) {
            serial_write("[M021] HID interface has missing/unsupported Report Descriptor length.\n");
            st.failure_stage=3;return false;
        }
        bool endpoint_found=false;
        for(uint8_t e=0;e<src->endpoint_count;++e) {
            uint8_t ix=(uint8_t)(src->first_endpoint+e);
            if(ix>=m19->endpoint_count) break;
            const psre_m019_endpoint_t *ep=&m19->endpoints[ix];
            if((ep->address&M021_ENDPOINT_IN) &&
               ((ep->attributes&3u)==M021_ENDPOINT_INTERRUPT)) {
                dst->interrupt_in_endpoint=ep->address;
                dst->max_packet_size=(uint16_t)(ep->max_packet_size&0x07FFu);
                dst->interval=ep->interval;
                endpoint_found=true;break;
            }
        }
        if(!endpoint_found || !dst->max_packet_size) {
            serial_write("[M021] HID interface has no valid Interrupt IN endpoint.\n");
            st.failure_stage=4;return false;
        }
        if(!get_report_descriptor(dst->interface_number,dst->report_descriptor_length)) {
            serial_write("[M021] GET_DESCRIPTOR(Report) failed.\n");
            st.failure_stage=5;return false;
        }
        if(!parse_report_descriptor(dst,report_buffer,dst->report_descriptor_length)) {
            serial_write("[M021] Report Descriptor malformed or unsupported.\n");
            st.failure_stage=6;return false;
        }
        dst->kind=determine_kind(dst);
        ++st.descriptors_read;
        switch(dst->kind) {
            case PSRE_HID_DEVICE_KEYBOARD: ++st.keyboard_count;break;
            case PSRE_HID_DEVICE_MOUSE: ++st.mouse_count;break;
            case PSRE_HID_DEVICE_KEYBOARD_MOUSE:
                ++st.keyboard_count;++st.mouse_count;break;
            default: ++st.other_count;break;
        }
    }
    if(!st.hid_interface_count) {
        serial_write("[M021] No HID interface found in active configuration.\n");
        st.failure_stage=7;return false;
    }
    st.success=true;
    serial_write("[M021] HID interface(s) parsed. Interrupt endpoints identified; input polling driver is not yet active.\n");
    return true;
}
static const char *kind_name(psre_hid_device_kind_t k) {
    switch(k) {
        case PSRE_HID_DEVICE_KEYBOARD:return "KEYBOARD";
        case PSRE_HID_DEVICE_MOUSE:return "MOUSE";
        case PSRE_HID_DEVICE_KEYBOARD_MOUSE:return "KEYBOARD+MOUSE";
        case PSRE_HID_DEVICE_OTHER:return "OTHER HID";
        default:return "UNKNOWN";
    }
}
void psre_m021_report(void) {
    serial_write("\n--- PSRE M021 HID FOUNDATION STATUS ---\n");
    serial_write("Attempted=");serial_write(st.attempted?"yes":"no");
    serial_write(" Success=");serial_write(st.success?"yes":"no");
    serial_write(" HID interfaces=");dec(st.hid_interface_count);
    serial_write(" Keyboard=");dec(st.keyboard_count);
    serial_write(" Mouse=");dec(st.mouse_count);
    serial_write(" Other=");dec(st.other_count);
    serial_write(" Failure stage=");dec(st.failure_stage);serial_write("\n");
    for(uint8_t i=0;i<st.hid_interface_count;++i) {
        const psre_m021_hid_interface_t *it=&st.interfaces[i];
        serial_write("IF ");dec(it->interface_number);
        serial_write(" subclass=");dec(it->subclass);
        serial_write(" protocol=");dec(it->protocol);
        serial_write(" interrupt-IN=0x");hex8(it->interrupt_in_endpoint);
        serial_write(" MPS=");dec(it->max_packet_size);
        serial_write(" interval=");dec(it->interval);
        serial_write(" report-length=");dec(it->report_descriptor_length);
        serial_write(" kind=");serial_write(kind_name(it->kind));serial_write("\n");
        for(uint8_t r=0;r<it->report_id_count;++r) {
            const psre_m021_report_id_t *rid=&it->report_ids[r];
            serial_write("  Report ID=");dec(rid->report_id);
            serial_write(" input-bits=");dec(rid->input_bits);
            serial_write(" input-bytes=");dec(rid->input_bytes);
            serial_write(" keyboard=");serial_write(rid->has_keyboard?"yes":"no");
            serial_write(" mouse=");serial_write(rid->has_mouse?"yes":"no");
            serial_write("\n");
        }
    }
    serial_write("[M021] HID descriptor discovery only; no Configure Endpoint, interrupt transfers, keyboard or mouse events yet.\n");
}
const psre_m021_status_t *psre_m021_get_status(void){return &st;}
