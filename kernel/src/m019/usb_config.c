#include <psre/m019/usb_config.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/serial.h>
#include <stdint.h>
#include <stdbool.h>

#define M019_DESC_CONFIGURATION 2u
#define M019_DESC_STRING 3u
#define M019_MAX_CONFIG_BYTES 1024u
#define M019_MAX_STRING_BYTES 126u

static psre_m019_status_t st;
static uint8_t config_data[M019_MAX_CONFIG_BYTES];

static void dec(uint32_t v) {
    char b[11]; unsigned n=0;
    if(!v){serial_write("0");return;}
    while(v && n<sizeof(b)){b[n++]=(char)('0'+v%10u);v/=10u;}
    while(n)serial_write_char(b[--n]);
}
static void hex8(uint8_t v) {
    static const char h[]="0123456789ABCDEF";
    serial_write_char(h[v>>4]); serial_write_char(h[v&15u]);
}
static bool get_descriptor(uint8_t type, uint8_t index, uint16_t lang,
                           uint16_t length, uint8_t *out, uint16_t cap) {
    return psre_m018_control_in(0x80u,6u,
        (uint16_t)(((uint16_t)type<<8)|index),lang,length,out,cap);
}
static uint8_t utf16le_to_ascii(const uint8_t *src, uint16_t bytes,
                                char *dst, uint8_t cap) {
    if(!src || !dst || cap<1u) return 0;
    uint8_t n=0;
    for(uint16_t i=2u;i+1u<bytes && n<cap-1u;i+=2u) {
        uint16_t ch=(uint16_t)(src[i]|((uint16_t)src[i+1u]<<8));
        dst[n++]=(ch>=32u && ch<=126u)?(char)ch:'?';
    }
    dst[n]=0;
    return n;
}
static bool read_string(uint8_t descriptor_index, uint16_t lang,
                        char *dst, uint8_t cap, uint8_t *out_len) {
    uint8_t tmp[M019_MAX_STRING_BYTES];
    if(!descriptor_index) { dst[0]=0; *out_len=0; return true; }
    if(!get_descriptor(M019_DESC_STRING,descriptor_index,lang,2u,tmp,sizeof(tmp)))
        return false;
    uint8_t len=tmp[0];
    if(len<2u || len>M019_MAX_STRING_BYTES || (len&1u))
        return false;
    if(!get_descriptor(M019_DESC_STRING,descriptor_index,lang,len,tmp,sizeof(tmp)))
        return false;
    *out_len=utf16le_to_ascii(tmp,len,dst,cap);
    return true;
}
static bool parse_configuration(uint16_t total) {
    uint16_t off=0;
    int current=-1;
    while(off<total) {
        if((uint16_t)(total-off)<(uint16_t)2u) return false;
        uint8_t len=config_data[off], type=config_data[off+1u];
        if(len<2u || len>total-off) return false;
        if(type==4u) {
            if(len<9u || st.interface_count>=PSRE_M019_MAX_INTERFACES)
                return false;
            current=(int)st.interface_count++;
            psre_m019_interface_t *it=&st.interfaces[current];
            it->number=config_data[off+2u];
            it->alternate_setting=config_data[off+3u];
            it->endpoint_count=config_data[off+4u];
            it->first_endpoint=st.endpoint_count;
            it->class_code=config_data[off+5u];
            it->subclass=config_data[off+6u];
            it->protocol=config_data[off+7u];
        } else if(type==0x21u) {
            if(current<0 || len<6u) return false;
            psre_m019_interface_t *it=&st.interfaces[current];
            if(it->class_code==0x03u) {
                uint8_t descriptors=config_data[off+5u];
                uint16_t pos=(uint16_t)(off+6u);
                for(uint8_t d=0;d<descriptors;++d) {
                    if(pos+3u>off+len) return false;
                    uint8_t dtype=config_data[pos];
                    uint16_t dlen=(uint16_t)(config_data[pos+1u]|
                        ((uint16_t)config_data[pos+2u]<<8));
                    if(dtype==0x22u) {
                        it->hid_descriptor_present=true;
                        it->hid_report_descriptor_length=dlen;
                    }
                    pos=(uint16_t)(pos+3u);
                }
            }
        } else if(type==5u) {
            if(len<7u || current<0 || st.endpoint_count>=PSRE_M019_MAX_ENDPOINTS)
                return false;
            psre_m019_endpoint_t *ep=&st.endpoints[st.endpoint_count++];
            ep->address=config_data[off+2u];
            ep->attributes=config_data[off+3u];
            ep->max_packet_size=(uint16_t)(config_data[off+4u]|
                ((uint16_t)config_data[off+5u]<<8));
            ep->interval=config_data[off+6u];
        }
        off=(uint16_t)(off+len);
    }
    return off==total;
}
bool psre_m019_read_configuration(void) {
    st=(psre_m019_status_t){0};
    st.attempted=true;
    const psre_m018_status_t *m18=psre_m018_get_status();
    if(!m18 || !m18->descriptor_read || !m18->configuration_count) {
        serial_write("[M019] Successful M018 Device Descriptor is required first.\n");
        st.failure_stage=1; return false;
    }
    uint8_t head[9];
    if(!get_descriptor(M019_DESC_CONFIGURATION,0u,0u,9u,head,sizeof(head))) {
        serial_write("[M019] Configuration Descriptor header request failed.\n");
        st.failure_stage=2; return false;
    }
    if(head[0]<9u || head[1]!=M019_DESC_CONFIGURATION) {
        serial_write("[M019] Invalid Configuration Descriptor header.\n");
        st.failure_stage=3; return false;
    }
    uint16_t total=(uint16_t)(head[2]|((uint16_t)head[3]<<8));
    if(total<9u || total>M019_MAX_CONFIG_BYTES) {
        serial_write("[M019] Configuration total length is outside 9..1024 bytes.\n");
        st.failure_stage=4; return false;
    }
    if(!get_descriptor(M019_DESC_CONFIGURATION,0u,0u,total,config_data,sizeof(config_data))) {
        serial_write("[M019] Full Configuration Descriptor request failed.\n");
        st.failure_stage=5; return false;
    }
    if(config_data[0]<9u || config_data[1]!=M019_DESC_CONFIGURATION ||
       (uint16_t)(config_data[2]|((uint16_t)config_data[3]<<8))!=total) {
        serial_write("[M019] Configuration Descriptor consistency check failed.\n");
        st.failure_stage=6; return false;
    }
    st.configuration_total_length=total;
    st.configuration_value=config_data[5];
    st.configuration_attributes=config_data[7];
    st.max_power_ma=(uint8_t)(config_data[8]*2u);
    if(!parse_configuration(total)) {
        serial_write("[M019] Malformed/unsupported interface or endpoint descriptor layout.\n");
        st.failure_stage=7; return false;
    }
    uint8_t lang_desc[126]; uint16_t lang=0x0409u;
    if(m18->manufacturer_index || m18->product_index || m18->serial_index) {
        if(get_descriptor(M019_DESC_STRING,0u,0u,2u,lang_desc,sizeof(lang_desc)) &&
           lang_desc[0]>=4u && lang_desc[1]==M019_DESC_STRING &&
           get_descriptor(M019_DESC_STRING,0u,0u,lang_desc[0],lang_desc,sizeof(lang_desc)))
            lang=(uint16_t)(lang_desc[2]|((uint16_t)lang_desc[3]<<8));
        if(!read_string(m18->manufacturer_index,lang,st.manufacturer,sizeof(st.manufacturer),&st.manufacturer_length) ||
           !read_string(m18->product_index,lang,st.product,sizeof(st.product),&st.product_length) ||
           !read_string(m18->serial_index,lang,st.serial,sizeof(st.serial),&st.serial_length)) {
            serial_write("[M019] String descriptor read failed; configuration parsed but strings incomplete.\n");
            st.failure_stage=8; return false;
        }
    }
    st.success=true;
    serial_write("[M019] Configuration parsed: interfaces=");dec(st.interface_count);
    serial_write(" endpoints=");dec(st.endpoint_count);
    serial_write(" total-length=");dec(st.configuration_total_length);serial_write("\n");
    return true;
}
void psre_m019_report(void) {
    serial_write("\n--- PSRE M019 USB CONFIGURATION STATUS ---\n");
    serial_write("Attempted=");serial_write(st.attempted?"yes":"no");
    serial_write(" Success=");serial_write(st.success?"yes":"no");
    serial_write(" Config value=");dec(st.configuration_value);
    serial_write(" Attributes=0x");hex8(st.configuration_attributes);
    serial_write(" MaxPower(mA)=");dec(st.max_power_ma);
    serial_write(" Length=");dec(st.configuration_total_length);
    serial_write(" Interfaces=");dec(st.interface_count);
    serial_write(" Endpoints=");dec(st.endpoint_count);
    serial_write(" Failure stage=");dec(st.failure_stage);serial_write("\n");
    for(uint8_t i=0;i<st.interface_count;++i) {
        const psre_m019_interface_t *it=&st.interfaces[i];
        serial_write(" IF ");dec(it->number);serial_write(" alt=");dec(it->alternate_setting);
        serial_write(" class=0x");hex8(it->class_code);serial_write(" subclass=0x");
        hex8(it->subclass);serial_write(" protocol=0x");hex8(it->protocol);
        serial_write(" declared EP=");dec(it->endpoint_count);
        if(it->hid_descriptor_present) {
            serial_write(" HID-report-length=");dec(it->hid_report_descriptor_length);
        }
        serial_write("\n");
    }
    for(uint8_t i=0;i<st.endpoint_count;++i) {
        const psre_m019_endpoint_t *ep=&st.endpoints[i];
        serial_write(" EP address=0x");hex8(ep->address);
        serial_write(" attributes=0x");hex8(ep->attributes);
        serial_write(" max-packet=");dec(ep->max_packet_size);
        serial_write(" interval=");dec(ep->interval);serial_write("\n");
    }
    serial_write("Manufacturer: ");serial_write(st.manufacturer);serial_write("\nProduct: ");
    serial_write(st.product);serial_write("\nSerial: ");serial_write(st.serial);serial_write("\n");
    serial_write("[M019] USB hubs, alternate-setting selection, class drivers and Mass Storage are not implemented.\n");
}
const psre_m019_status_t *psre_m019_get_status(void){return &st;}
