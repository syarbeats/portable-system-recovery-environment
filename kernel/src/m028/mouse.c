#include <psre/m028/mouse.h>
#include <psre/m021/hid.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/serial.h>
#include <stdint.h>
#include <stdbool.h>

#define M028_DESC_REPORT 0x22u
#define M028_USAGE_PAGE_BUTTON 0x09u
#define M028_USAGE_PAGE_GENERIC 0x01u
#define M028_USAGE_PAGE_CONSUMER 0x0Cu
#define M028_USAGE_X 0x30u
#define M028_USAGE_Y 0x31u
#define M028_USAGE_WHEEL 0x38u
#define M028_USAGE_AC_PAN 0x0238u
#define M028_SPAN_STACK 4u

typedef struct {
    uint8_t report_id;
    uint16_t usage_page, usage_min, usage_max;
    uint16_t bit_offset;
    uint8_t report_size, report_count;
    int32_t logical_min, logical_max;
    uint8_t flags;
} m028_field_t;
typedef struct {
    uint8_t report_id;
    uint16_t input_bits;
} m028_report_t;
typedef struct {
    uint32_t usage_page, report_size, report_count;
    int32_t logical_min, logical_max;
    uint8_t report_id;
} m028_global_t;

static psre_m028_status_t st;
static m028_field_t fields[PSRE_M028_MAX_FIELDS];
static m028_report_t reports[PSRE_M028_MAX_REPORT_IDS];
static uint8_t field_count, report_count;
static uint8_t descriptor[PSRE_M028_MAX_REPORT_DESCRIPTOR];

static void dec(uint32_t v) { char b[11]; unsigned n=0; if(!v){serial_write("0");return;} while(v&&n<sizeof(b)){b[n++]=(char)('0'+v%10u);v/=10u;} while(n)serial_write_char(b[--n]); }
static void hex8(uint8_t v) { static const char h[]="0123456789ABCDEF"; serial_write_char(h[v>>4]); serial_write_char(h[v&15u]); }
static int32_t signed_value(uint32_t v, uint8_t bytes) {
    if(bytes==0u) return 0;
    if(bytes>=4u) return (int32_t)v;
    uint32_t bits=(uint32_t)bytes*8u;
    uint32_t sign=1u<<(bits-1u);
    uint32_t mask=(1u<<bits)-1u;
    v&=mask;
    return (v&sign)?(int32_t)(v|~mask):(int32_t)v;
}
static int32_t sign_extend_bits(uint32_t v,uint8_t bits) {
    if(!bits || bits>=32u) return (int32_t)v;
    uint32_t sign=1u<<((uint32_t)bits-1u);
    uint32_t mask=(1u<<(uint32_t)bits)-1u;
    v&=mask;
    return (v&sign)?(int32_t)(v|~mask):(int32_t)v;
}
static int find_report(uint8_t id) {
    for(uint8_t i=0;i<report_count;++i) if(reports[i].report_id==id) return (int)i;
    if(report_count>=PSRE_M028_MAX_REPORT_IDS) return -1;
    uint8_t n=report_count++; reports[n]=(m028_report_t){id,0}; return (int)n;
}
static int lookup_report(uint8_t id) {
    for(uint8_t i=0;i<report_count;++i) if(reports[i].report_id==id) return (int)i;
    return -1;
}
static bool add_input(uint32_t usage_page,uint32_t usage_min,uint32_t usage_max,
                      uint32_t size,uint32_t count,int32_t logical_min,
                      int32_t logical_max,uint8_t flags,uint8_t report_id,
                      bool in_mouse_collection) {
    int ri=find_report(report_id);
    if(ri<0 || size>32u || count>255u || (size && count>65535u/size)) return false;
    uint32_t bits=size*count;
    if(bits>65535u-reports[ri].input_bits) return false;
    uint16_t offset=reports[ri].input_bits;
    reports[ri].input_bits=(uint16_t)(offset+bits);
    if(!in_mouse_collection || (flags&1u) || !size || !count) return true;
    bool relevant=(usage_page==M028_USAGE_PAGE_BUTTON) ||
                  (usage_page==M028_USAGE_PAGE_GENERIC &&
                   (usage_min==M028_USAGE_X || usage_min==M028_USAGE_Y || usage_min==M028_USAGE_WHEEL)) ||
                  (usage_page==M028_USAGE_PAGE_CONSUMER && usage_min==M028_USAGE_AC_PAN);
    if(!relevant) { if(in_mouse_collection && !(flags&1u)) ++st.unsupported_fields; return true; }
    if(!(flags&2u) || (usage_page!=M028_USAGE_PAGE_BUTTON && !(flags&4u))) {
        ++st.unsupported_fields; return true;
    }
    if(field_count>=PSRE_M028_MAX_FIELDS) return false;
    m028_field_t *f=&fields[field_count++];
    f->report_id=report_id; f->usage_page=(uint16_t)usage_page;
    f->usage_min=(uint16_t)usage_min; f->usage_max=(uint16_t)(usage_max?usage_max:usage_min);
    f->bit_offset=offset; f->report_size=(uint8_t)size; f->report_count=(uint8_t)count;
    f->logical_min=logical_min; f->logical_max=logical_max; f->flags=flags;
    return true;
}
static bool parse_descriptor(const uint8_t *data,uint16_t length) {
    m028_global_t g={0}, stack[M028_SPAN_STACK]; uint8_t stack_depth=0;
    uint32_t local_usage=0, usage_min=0, usage_max=0;
    uint8_t collection_depth=0, mouse_depth=0;
    uint16_t pos=0;
    field_count=0; report_count=0;
    while(pos<length) {
        uint8_t prefix=data[pos++];
        if(prefix==0xFEu) {
            if((uint16_t)(length-pos)<2u) return false;
            uint8_t n=data[pos++]; ++pos;
            if(n>(uint16_t)(length-pos)) return false;
            pos=(uint16_t)(pos+n); continue;
        }
        uint8_t sc=(uint8_t)(prefix&3u), size=(sc==3u)?4u:sc;
        uint8_t type=(uint8_t)((prefix>>2)&3u), tag=(uint8_t)(prefix>>4);
        if(size>(uint16_t)(length-pos)) return false;
        uint32_t v=0; for(uint8_t i=0;i<size;++i) v|=(uint32_t)data[pos+i]<<(8u*i); pos=(uint16_t)(pos+size);
        if(type==1u) {
            if(tag==0u) g.usage_page=v;
            else if(tag==1u) g.logical_min=signed_value(v,size);
            else if(tag==2u) g.logical_max=(g.logical_min<0)?signed_value(v,size):(int32_t)v;
            else if(tag==7u) g.report_size=v;
            else if(tag==8u) { if(!v||v>255u)return false; g.report_id=(uint8_t)v; st.has_report_ids=true; }
            else if(tag==9u) g.report_count=v;
            else if(tag==10u) { if(stack_depth>=M028_SPAN_STACK)return false; stack[stack_depth++]=g; }
            else if(tag==11u) { if(!stack_depth)return false; g=stack[--stack_depth]; }
        } else if(type==2u) {
            if(tag==0u) {
                if(!usage_min) usage_min=v;
                usage_max=v;
                local_usage=v;
            } else if(tag==1u) usage_min=v;
            else if(tag==2u) usage_max=v;
        } else if(type==0u) {
            if(tag==10u) { /* Collection */
                uint32_t u=local_usage?local_usage:usage_min;
                if(collection_depth==255u) return false;
                if(g.usage_page==M028_USAGE_PAGE_GENERIC && u==0x02u) mouse_depth=(uint8_t)(collection_depth+1u);
                ++collection_depth;
                local_usage=usage_min=usage_max=0;
            } else if(tag==12u) { /* End Collection */
                if(collection_depth) --collection_depth;
                if(mouse_depth>collection_depth) mouse_depth=0;
                local_usage=usage_min=usage_max=0;
            } else if(tag==8u) { /* Input */
                if(!add_input(g.usage_page,usage_min?usage_min:local_usage,
                              usage_max?usage_max:(local_usage?local_usage:usage_min),
                              g.report_size,g.report_count,g.logical_min,g.logical_max,
                              (uint8_t)v,g.report_id,mouse_depth!=0u)) return false;
                local_usage=usage_min=usage_max=0;
            } else if(tag==9u || tag==11u) local_usage=usage_min=usage_max=0;
        }
    }
    if(collection_depth || stack_depth || !report_count || !field_count) return false;
    return true;
}
static uint32_t extract_bits(const uint8_t *p,uint16_t offset,uint8_t width) {
    uint32_t value=0;
    for(uint8_t b=0;b<width;++b) if(p[(offset+b)>>3]&(uint8_t)(1u<<((offset+b)&7u))) value|=1u<<b;
    return value;
}
static int16_t clamp_i16(int32_t v) { if(v>32767)return 32767; if(v< -32768)return -32768; return (int16_t)v; }
static int8_t clamp_i8(int32_t v) { if(v>127)return 127; if(v< -128)return -128; return (int8_t)v; }
static void signed8(int8_t v) { if(v<0){serial_write_char('-');dec((uint8_t)(-(int16_t)v));}else dec((uint8_t)v); }

bool psre_m028_initialize(void) {
    st=(psre_m028_status_t){0}; field_count=report_count=0;
    const psre_m021_status_t *hid=psre_m021_get_status();
    const psre_m018_status_t *dev=psre_m018_get_status();
    if(!hid||!hid->success||!dev||!dev->address_assigned) { serial_write("[M028] M018 enumeration and M021 HID probe are required.\n"); return false; }
    const psre_m021_hid_interface_t *chosen=0;
    for(uint8_t i=0;i<hid->hid_interface_count;++i) {
        const psre_m021_hid_interface_t *it=&hid->interfaces[i];
        if(it->present && it->kind==PSRE_HID_DEVICE_MOUSE && it->interrupt_in_endpoint && it->report_descriptor_length<=PSRE_M028_MAX_REPORT_DESCRIPTOR) { chosen=it; break; }
    }
    if(!chosen) { serial_write("[M028] No dedicated HID mouse interface found; composite keyboard+mouse interfaces are not yet supported.\n"); return false; }
    if(!psre_m018_control_in(0x81u,6u,(uint16_t)((M028_DESC_REPORT<<8)|0u),chosen->interface_number,
                             chosen->report_descriptor_length,descriptor,sizeof(descriptor))) {
        ++st.descriptor_errors; serial_write("[M028] GET_DESCRIPTOR(Report) failed.\n"); return false;
    }
    st.descriptor_length=chosen->report_descriptor_length;
    if(!parse_descriptor(descriptor,st.descriptor_length)) { ++st.descriptor_errors; serial_write("[M028] Report Descriptor parse failed or exceeded limits.\n"); return false; }
    for(uint8_t i=0;i<report_count;++i) {
        uint16_t bytes=(uint16_t)((reports[i].input_bits+7u)/8u);
        if(st.has_report_ids) ++bytes;
        if(bytes>chosen->max_packet_size) { ++st.descriptor_errors; serial_write("[M028] Parsed input report exceeds endpoint maximum packet size.\n"); return false; }
    }
    if(chosen->subclass==1u) {
        if(!psre_m018_control_no_data(0x21u,0x0Bu,1u,chosen->interface_number)) {
            serial_write("[M028] SET_PROTOCOL(Report) failed.\n"); return false;
        }
    }
    st.interface_number=chosen->interface_number; st.endpoint_address=chosen->interrupt_in_endpoint;
    st.max_packet_size=chosen->max_packet_size; st.interval=chosen->interval;
    if(!st.max_packet_size || st.max_packet_size>1024u || !st.interval) return false;
    st.report_protocol=true; st.initialized=true;
    serial_write("[M028] HID Report Protocol mouse initialized (wheel + buttons 1..8 where described).\n");
    return true;
}

bool psre_m028_process_report(const uint8_t *r,uint16_t n) {
    if(!st.initialized || !r || !n) { ++st.malformed_reports; return false; }
    uint8_t id=0; const uint8_t *payload=r; uint16_t payload_len=n;
    if(st.has_report_ids) { id=r[0]; payload=&r[1]; --payload_len; }
    int ri=lookup_report(id);
    if(ri<0) { ++st.malformed_reports; return false; }
    uint16_t required=(uint16_t)((reports[ri].input_bits+7u)/8u);
    if(payload_len<required) { ++st.malformed_reports; return false; }
    int32_t x=0,y=0,wheel=0,pan=0; uint8_t buttons=0;
    for(uint8_t i=0;i<field_count;++i) {
        const m028_field_t *f=&fields[i]; if(f->report_id!=id || !(f->flags&2u)) continue;
        for(uint8_t j=0;j<f->report_count;++j) {
            uint16_t off=(uint16_t)(f->bit_offset+(uint16_t)j*f->report_size);
            uint32_t raw=extract_bits(payload,off,f->report_size);
            int32_t value=(f->logical_min<0)?sign_extend_bits(raw,f->report_size):(int32_t)raw;
            uint32_t usage=(uint32_t)f->usage_min+j;
            if(f->usage_page==M028_USAGE_PAGE_BUTTON) {
                if(usage>=1u && usage<=8u && value) buttons|=(uint8_t)(1u<<(usage-1u));
            } else if(f->usage_page==M028_USAGE_PAGE_GENERIC) {
                if(usage==M028_USAGE_X)x+=value; else if(usage==M028_USAGE_Y)y+=value; else if(usage==M028_USAGE_WHEEL)wheel+=value;
            } else if(f->usage_page==M028_USAGE_PAGE_CONSUMER && usage==M028_USAGE_AC_PAN) pan+=value;
        }
    }
    st.buttons=buttons; st.delta_x=clamp_i16(x); st.delta_y=clamp_i16(y);
    st.wheel=clamp_i8(wheel); st.horizontal_wheel=clamp_i8(pan); ++st.reports;
    return true;
}
const psre_m028_status_t *psre_m028_get_status(void) { return &st; }
void psre_m028_report(void) {
    serial_write("\n--- M028 ADVANCED USB HID MOUSE ---\nInitialized="); serial_write(st.initialized?"yes":"no");
    serial_write(" IF="); dec(st.interface_number); serial_write(" EP=0x"); hex8(st.endpoint_address);
    serial_write(" MPS="); dec(st.max_packet_size); serial_write(" protocol="); serial_write(st.report_protocol?"Report":"Boot");
    serial_write(" buttons="); dec(st.buttons); serial_write(" dx="); dec((uint16_t)st.delta_x);
    serial_write(" dy="); dec((uint16_t)st.delta_y); serial_write(" wheel="); signed8(st.wheel);
    serial_write(" pan="); signed8(st.horizontal_wheel); serial_write(" reports="); dec(st.reports);
    serial_write(" malformed="); dec(st.malformed_reports); serial_write(" descriptor-errors="); dec(st.descriptor_errors);
    serial_write(" fields="); dec(field_count); serial_write(" unsupported-fields="); dec(st.unsupported_fields); serial_write("\n");
}
