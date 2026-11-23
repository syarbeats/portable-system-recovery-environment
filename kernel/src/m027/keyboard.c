#include <psre/m027/keyboard.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/serial.h>

static psre_m027_status_t st;
static uint8_t previous[6];
static uint8_t last_repeat_key;

static void dec(uint32_t v) {
    char b[10]; unsigned n=0;
    if (!v) { serial_write("0"); return; }
    while (v && n<sizeof(b)) { b[n++]=(char)('0'+v%10u); v/=10u; }
    while (n) serial_write_char(b[--n]);
}
static bool has_key(const uint8_t *keys, uint8_t key) {
    for (uint8_t i=0;i<6u;++i) if (keys[i]==key && key) return true;
    return false;
}
static void set_bit(uint8_t key, bool down) {
    uint8_t mask=(uint8_t)(1u<<(key&7u));
    if (down) st.key_bitmap[key>>3]|=mask;
    else st.key_bitmap[key>>3]&=(uint8_t)~mask;
}
static bool is_down(uint8_t key) {
    return (st.key_bitmap[key>>3]&(uint8_t)(1u<<(key&7u)))!=0u;
}
static bool send_led_report(void) {
    /* HID SET_REPORT(Output), report ID 0, one-byte LED bitmap. */
    uint8_t value=st.leds;
    return psre_m018_control_out(0x21u,0x09u,0x0200u,
                                 st.interface_number,1u,&value,1u);
}
static void update_led(uint8_t key) {
    uint8_t bit;
    if (key==0x39u) bit=1u;       /* Caps Lock */
    else if (key==0x53u) bit=0u;  /* Num Lock */
    else if (key==0x47u) bit=2u;  /* Scroll Lock */
    else return;
    st.leds^=(uint8_t)(1u<<bit);
    if (send_led_report()) ++st.led_updates;
    else ++st.led_errors;
}
bool psre_m027_initialize(uint8_t interface_number) {
    st=(psre_m027_status_t){0};
    for (uint8_t i=0;i<6u;++i) previous[i]=0;
    last_repeat_key=0; 
    st.interface_number=interface_number;
    st.initialized=true;
    serial_write("[M027] Advanced Boot Keyboard state initialized (6KRO, repeat, LEDs).\n");
    return true;
}
bool psre_m027_process_report(const uint8_t *r,uint16_t n,
                              uint64_t now,uint8_t *pressed_key) {
    if (pressed_key) *pressed_key=0;
    if (!st.initialized || !r || n<8u) return false;
    /* HID ErrorRollOver (1) means the key array is not authoritative. */
    for (uint8_t i=0;i<6u;++i) if (r[i+2u]>=1u && r[i+2u]<=3u) {
        ++st.rollover_reports; return false;
    }
    for (uint8_t bit=0;bit<8u;++bit) {
        uint8_t usage=(uint8_t)(0xE0u+bit);
        set_bit(usage,(r[0]&(uint8_t)(1u<<bit))!=0u);
    }
    for (uint8_t i=0;i<6u;++i) {
        uint8_t key=r[i+2u];
        if (key && !has_key(previous,key)) {
            set_bit(key,true);
            update_led(key);
            if (key<0xE0u) {
                last_repeat_key=key; st.repeat_key=key; 
                st.repeat_deadline=now+PSRE_M027_REPEAT_DELAY_TICKS;
            }
            if (pressed_key && !*pressed_key) *pressed_key=key;
        }
    }
    for (uint8_t i=0;i<6u;++i) {
        uint8_t key=previous[i];
        if (key && !has_key(&r[2],key)) {
            set_bit(key,false);
            if (key==last_repeat_key) { last_repeat_key=0; st.repeat_key=0; st.repeat_deadline=0; }
        }
        previous[i]=r[i+2u];
    }
    st.modifiers=r[0]; ++st.reports;
    return true;
}
bool psre_m027_repeat_due(uint64_t now,uint8_t *key) {
    if (!st.initialized || !key || !last_repeat_key ||
        !is_down(last_repeat_key) || now<st.repeat_deadline) return false;
    *key=last_repeat_key;
    ++st.repeat_events;
    st.repeat_deadline=now+PSRE_M027_REPEAT_INTERVAL_TICKS;
    
    return true;
}
const psre_m027_status_t *psre_m027_get_status(void) { return &st; }
void psre_m027_report(void) {
    serial_write("\n--- M027 ADVANCED USB KEYBOARD ---\nInitialized=");
    serial_write(st.initialized?"yes":"no");
    serial_write(" IF=");dec(st.interface_number);
    serial_write(" modifiers=");dec(st.modifiers);
    serial_write(" LED-mask=");dec(st.leds);
    serial_write(" repeat-key=");dec(st.repeat_key);
    serial_write(" reports=");dec(st.reports);
    serial_write(" rollover=");dec(st.rollover_reports);
    serial_write(" repeats=");dec(st.repeat_events);
    serial_write(" LED-updates=");dec(st.led_updates);
    serial_write(" LED-errors=");dec(st.led_errors);
    serial_write("\n[M027] Boot protocol supports 6KRO; report-protocol NKRO is not enabled.\n");
}
