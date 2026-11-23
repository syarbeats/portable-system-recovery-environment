#include <psre/m053/hub_driver.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/m020/usb_device.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
static psre_m018_status_t dev;
static psre_m020_status_t cfg;
static unsigned power_calls, reset_calls, clear_conn_calls, clear_reset_calls, status_calls;
static uint8_t last_type, last_req; static uint16_t last_value, last_index, last_len;
void serial_write(const char *s) {(void)s;}
void serial_write_char(char c) {(void)c;}
const psre_m018_status_t *psre_m018_get_status(void) { return &dev; }
const psre_m020_status_t *psre_m020_get_status(void) { return &cfg; }
bool psre_m018_control_in(uint8_t type,uint8_t req,uint16_t value,uint16_t index,uint16_t len,uint8_t *out,uint16_t cap) {
    last_type=type;last_req=req;last_value=value;last_index=index;last_len=len;
    if(!out || cap<len) return false;
    if(type==0xA0 && req==6 && len==7) { const uint8_t d[]={9,0x29,3,9,0,5,0}; memcpy(out,d,7); return true; }
    if(type==0xA0 && req==6 && len==9) { const uint8_t d[]={9,0x29,3,9,0,5,0,0,0}; memcpy(out,d,9); return true; }
    if(type==0xA3 && req==0 && len==4) { const uint8_t d[]={1,0,1,0}; memcpy(out,d,4); ++status_calls; return true; }
    return false;
}
bool psre_m018_control_no_data(uint8_t type,uint8_t req,uint16_t value,uint16_t index) {
    last_type=type;last_req=req;last_value=value;last_index=index;last_len=0;
    if(type==0x23 && req==3 && value==PSRE_M053_FEATURE_PORT_POWER) ++power_calls;
    else if(type==0x23 && req==3 && value==PSRE_M053_FEATURE_PORT_RESET) ++reset_calls;
    else if(type==0x23 && req==1 && value==PSRE_M053_FEATURE_C_PORT_CONNECTION) ++clear_conn_calls;
    else if(type==0x23 && req==1 && value==PSRE_M053_FEATURE_C_PORT_RESET) ++clear_reset_calls;
    else return false;
    return true;
}
int main(void) {
    dev=(psre_m018_status_t){.address_assigned=true,.descriptor_read=true,.port=2};
    cfg=(psre_m020_status_t){.configured=true,.hub_interfaces=1};
    assert(psre_m052_initialize()); assert(psre_m053_initialize());
    assert(psre_m053_probe_active_hub(1,0,0));
    assert(last_type==0x23 && power_calls==3);
    const psre_m053_status_t *s=psre_m053_get_status();
    assert(s->hub_probed && s->port_count==3 && s->ports_powered==3);
    psre_m053_port_status_t ps;
    assert(psre_m053_get_port_status(2,&ps));
    assert(ps.status==1 && ps.change==1 && last_type==0xA3 && last_index==2);
    assert(psre_m053_reset_port(2) && reset_calls==1);
    assert(psre_m053_clear_port_change(2,PSRE_M053_FEATURE_C_PORT_CONNECTION) && clear_conn_calls==1);
    assert(psre_m053_clear_port_change(2,PSRE_M053_FEATURE_C_PORT_RESET) && clear_reset_calls==1);
    assert(psre_m053_scan_ports());
    assert(status_calls==4 && clear_conn_calls==4 && reset_calls==4);
    assert(psre_m052_get_hub(1) && psre_m052_get_hub(1)->port_count==3);
    puts("[PASS] M053 Hub descriptor, port power/status/reset/change handling");
    return 0;
}
