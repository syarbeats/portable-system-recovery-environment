#include <psre/m052/usb_hub.h>
#include <assert.h>
#include <stdio.h>
void serial_write(const char *s) {(void)s;}
int main(void) {
    psre_m052_hub_descriptor_t d;
    const uint8_t desc[]={9,0x29,4,0x09,0x00,10,0,0,0};
    assert(psre_m052_parse_hub_descriptor(desc,sizeof desc,&d));
    assert(d.port_count==4 && d.bitmap_bytes==1 && d.characteristics==9 && d.power_good_2ms==10);
    const uint8_t bad[]={7,0x29,4,0,0,1,0};
    assert(!psre_m052_parse_hub_descriptor(bad,sizeof bad,&d));
    assert(!psre_m052_parse_hub_descriptor(desc,5,&d));
    assert(psre_m052_initialize());
    assert(psre_m052_register_hub(1,0,0,4,9));
    assert(psre_m052_register_hub(2,1,2,3,0));
    assert(!psre_m052_register_hub(3,99,1,2,0));
    const uint8_t bitmap[]={0x0A}; /* downstream ports 2 and 4 changed */
    assert(psre_m052_process_change_bitmap(1,bitmap,sizeof bitmap));
    assert(!psre_m052_process_change_bitmap(1,bitmap,0));
    assert(psre_m052_add_device(1,1,2,4,1,0x1234,0x5678));
    assert(psre_m052_add_device(2,0,0,5,3,0xABCD,0xEF01));
    assert(!psre_m052_add_device(1,1,2,4,1,0,0));
    assert(psre_m052_get_device(1)->depth==1);
    assert(psre_m052_remove_device(1));
    assert(!psre_m052_remove_device(1));
    psre_m052_event_t e; unsigned n=0;
    while(psre_m052_next_event(&e)) ++n;
    assert(n==7);
    const psre_m052_status_t *s=psre_m052_get_status();
    assert(s->hub_count==2 && s->device_count==1 && s->port_events==2 && s->device_adds==2 && s->device_removes==1);
    puts("[PASS] M052 hub descriptor, topology, bitmap and multi-device registry tests");
    return 0;
}
