#include <psre/m025/input.h>
#include <psre/m023/keyboard.h>
#include <psre/m024/mouse.h>
#include <psre/m028/mouse.h>
#include <psre/m027/keyboard.h>
#include <psre/m029/layout.h>
#include <psre/m003/pit.h>
#include <psre/m022/interrupt.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/serial.h>
static psre_m025_status_t st;
static psre_input_event_t queue[PSRE_M025_EVENT_CAPACITY];
static uint16_t head,tail,count;
static uint8_t previous_keys[6],previous_modifiers,previous_buttons;
static bool mouse_uses_boot_protocol;
static uint8_t keyboard_report[1024],mouse_report[1024];
static void push(psre_input_event_t e){
    /* Coalesce consecutive relative moves (same buttons): one render per batch instead of per report. */
    if(e.type==PSRE_INPUT_MOUSE_MOVE&&count){
        psre_input_event_t *l=&queue[(uint16_t)((tail+PSRE_M025_EVENT_CAPACITY-1u)%PSRE_M025_EVENT_CAPACITY)];
        if(l->type==PSRE_INPUT_MOUSE_MOVE&&l->buttons==e.buttons){
            int32_t dx=(int32_t)l->delta_x+e.delta_x,dy=(int32_t)l->delta_y+e.delta_y;
            if(dx<INT16_MIN)dx=INT16_MIN;
            if(dx>INT16_MAX)dx=INT16_MAX;
            if(dy<INT16_MIN)dy=INT16_MIN;
            if(dy>INT16_MAX)dy=INT16_MAX;
            l->delta_x=(int16_t)dx;l->delta_y=(int16_t)dy;return;
        }
    }
    if(count>=PSRE_M025_EVENT_CAPACITY){++st.events_dropped;return;}queue[tail]=e;tail=(uint16_t)((tail+1u)%PSRE_M025_EVENT_CAPACITY);++count;++st.events_generated;st.queue_count=count;}
static bool contains(const uint8_t *keys,uint8_t key){for(uint8_t i=0;i<6u;++i)if(keys[i]==key&&key)return true;return false;}
static void key_event(uint8_t key,bool down,uint8_t modifiers){
    psre_input_event_t e={0}; e.type=down?PSRE_INPUT_KEY_DOWN:PSRE_INPUT_KEY_UP;
    e.device=PSRE_INPUT_DEVICE_KEYBOARD; e.code=key; e.modifiers=modifiers;
    const psre_m027_status_t *ks=psre_m027_get_status();
    e.character=down?psre_m029_translate(key,modifiers,ks?ks->leds:0u):0;
    push(e);
}
static void process_keyboard(const uint8_t *r,uint16_t n){if(!psre_m023_process_report(r,n))return;uint8_t newly_pressed=0;(void)psre_m027_process_report(r,n,psre_pit_ticks(),&newly_pressed);const psre_m023_status_t *k=psre_m023_get_status();for(uint8_t i=0;i<6u;++i)if(k->keys[i]==1u)return;for(uint8_t bit=0;bit<8u;++bit){uint8_t mask=(uint8_t)(1u<<bit);if((k->modifiers&mask)!=(previous_modifiers&mask))key_event((uint8_t)(0xE0u+bit),(k->modifiers&mask)!=0u,k->modifiers);}for(uint8_t i=0;i<6u;++i)if(previous_keys[i]&&!contains(k->keys,previous_keys[i]))key_event(previous_keys[i],false,k->modifiers);for(uint8_t i=0;i<6u;++i)if(k->keys[i]&&!contains(previous_keys,k->keys[i]))key_event(k->keys[i],true,k->modifiers);for(uint8_t i=0;i<6u;++i)previous_keys[i]=k->keys[i];previous_modifiers=k->modifiers;}
static void process_mouse(const uint8_t *r,uint16_t n){
    uint8_t buttons=0; int16_t dx=0,dy=0; int8_t wheel=0,hwheel=0;
    if(mouse_uses_boot_protocol) {
        if(!psre_m024_process_report(r,n)) return;
        const psre_m024_status_t *m=psre_m024_get_status();
        buttons=m->buttons; dx=m->delta_x; dy=m->delta_y; wheel=m->wheel;
    } else {
        if(!psre_m028_process_report(r,n)) return;
        const psre_m028_status_t *m=psre_m028_get_status();
        buttons=m->buttons; dx=m->delta_x; dy=m->delta_y;
        wheel=m->wheel; hwheel=m->horizontal_wheel;
    }
    if(dx||dy){psre_input_event_t e={0};e.type=PSRE_INPUT_MOUSE_MOVE;e.device=PSRE_INPUT_DEVICE_MOUSE;e.delta_x=dx;e.delta_y=dy;e.buttons=buttons;push(e);}
    for(uint8_t bit=0;bit<8u;++bit){uint8_t mask=(uint8_t)(1u<<bit);if((buttons&mask)!=(previous_buttons&mask)){psre_input_event_t e={0};e.type=(buttons&mask)?PSRE_INPUT_MOUSE_BUTTON_DOWN:PSRE_INPUT_MOUSE_BUTTON_UP;e.device=PSRE_INPUT_DEVICE_MOUSE;e.code=(uint8_t)(bit+1u);e.buttons=buttons;push(e);}}
    if(wheel){psre_input_event_t e={0};e.type=PSRE_INPUT_MOUSE_WHEEL;e.device=PSRE_INPUT_DEVICE_MOUSE;e.wheel=wheel;e.buttons=buttons;push(e);}
    if(hwheel){psre_input_event_t e={0};e.type=PSRE_INPUT_MOUSE_HWHEEL;e.device=PSRE_INPUT_DEVICE_MOUSE;e.horizontal_wheel=hwheel;e.buttons=buttons;push(e);}
    previous_buttons=buttons;
}
static uint8_t keyboard_slot,mouse_slot;
/* Add the HID device currently held by M018/M021. Safe to call once per enumerated
   device: an already-active keyboard or mouse is never re-initialized. */
bool psre_m025_add_device(void){
    const psre_m018_status_t *dev=psre_m018_get_status();uint8_t slot=dev?dev->slot_id:0;
    if(!st.keyboard_active){
        bool keyboard=psre_m023_initialize();const psre_m023_status_t *k=psre_m023_get_status();
        if(keyboard&&k&&k->initialized){(void)psre_m027_initialize(k->interface_number);
            if(psre_m022_configure_interrupt_in_multi(k->endpoint_address,k->max_packet_size,k->interval)){st.keyboard_active=true;keyboard_slot=slot;st.interface_number=k->interface_number;st.endpoint_address=k->endpoint_address;}
            else serial_write("[M026] Keyboard endpoint configuration failed.\n");}
    }
    if(!st.mouse_active){
        bool boot=false;bool mouse=psre_m028_initialize();
        if(!mouse){mouse=psre_m024_initialize();boot=mouse;}
        if(mouse){
            mouse_uses_boot_protocol=boot;
            uint8_t ep=boot?psre_m024_get_status()->endpoint_address:psre_m028_get_status()->endpoint_address;
            uint16_t mps=boot?psre_m024_get_status()->max_packet_size:psre_m028_get_status()->max_packet_size;
            uint8_t iv=boot?psre_m024_get_status()->interval:psre_m028_get_status()->interval;
            if(psre_m022_configure_interrupt_in_multi(ep,mps,iv)){st.mouse_active=true;mouse_slot=slot;if(!st.keyboard_active){st.interface_number=boot?psre_m024_get_status()->interface_number:psre_m028_get_status()->interface_number;st.endpoint_address=ep;}}
            else serial_write("[M025] Mouse endpoint configuration failed.\n");
        }
    }
    st.endpoint_configured=st.keyboard_active||st.mouse_active;st.initialized=st.endpoint_configured;
    st.active_device=st.keyboard_active?PSRE_INPUT_DEVICE_KEYBOARD:st.mouse_active?PSRE_INPUT_DEVICE_MOUSE:PSRE_INPUT_DEVICE_NONE;
    serial_write(st.keyboard_active&&st.mouse_active?"[M026] Keyboard and mouse endpoints active.\n":st.keyboard_active?"[M026] Keyboard endpoint active; mouse unavailable.\n":st.mouse_active?"[M026] Mouse endpoint active; keyboard unavailable.\n":"[M026] No supported HID boot input endpoint.\n");
    return st.initialized;
}
bool psre_m025_initialize(void){st=(psre_m025_status_t){0};head=tail=count=0;previous_modifiers=previous_buttons=0;for(uint8_t i=0;i<6u;++i)previous_keys[i]=0;mouse_uses_boot_protocol=false;keyboard_slot=mouse_slot=0;return psre_m025_add_device();}
bool psre_m025_poll(uint32_t timeout){if(!st.initialized||!timeout)return false;bool any=false;++st.polls;
    if(st.keyboard_active){const psre_m023_status_t *k=psre_m023_get_status();uint16_t n=0;uint16_t cap=k->max_packet_size<=sizeof(keyboard_report)?k->max_packet_size:0;if(cap&&psre_m022_interrupt_in_slot(keyboard_slot,k->endpoint_address,keyboard_report,cap,&n,0u)){++st.keyboard_polls;process_keyboard(keyboard_report,n);any=true;}else ++st.transfer_errors;}
    if(st.mouse_active){
        uint8_t endpoint=mouse_uses_boot_protocol?psre_m024_get_status()->endpoint_address:psre_m028_get_status()->endpoint_address;
        uint16_t mps=mouse_uses_boot_protocol?psre_m024_get_status()->max_packet_size:psre_m028_get_status()->max_packet_size;
        uint16_t n=0;uint16_t cap=mps<=sizeof(mouse_report)?mps:0;
        /* Non-blocking: events are collected from the xHCI event ring, so never spin 10 ms per
           endpoint. Drain up to 16 queued reports so cursor speed is not capped at the poll rate. */
        bool got_mouse=false;
        for(uint8_t burst=0;cap&&burst<16u;++burst){
            if(!psre_m022_interrupt_in_slot(mouse_slot,endpoint,mouse_report,cap,&n,0u))break;
            ++st.mouse_polls;process_mouse(mouse_report,n);got_mouse=true;any=true;
        }
        if(!got_mouse)++st.transfer_errors;
    }
    uint8_t repeat_key=0;
    if(st.keyboard_active && psre_m027_repeat_due(psre_pit_ticks(),&repeat_key)) {
        psre_input_event_t e={0}; e.type=PSRE_INPUT_KEY_REPEAT;
        e.device=PSRE_INPUT_DEVICE_KEYBOARD; e.code=repeat_key;
        const psre_m027_status_t *k=psre_m027_get_status();
        e.modifiers=k?k->modifiers:0; e.character=psre_m029_translate(repeat_key,e.modifiers,k?k->leds:0u); push(e); any=true;
    }
    return any;
}
bool psre_m025_next_event(psre_input_event_t *e){if(!e||!count)return false;*e=queue[head];head=(uint16_t)((head+1u)%PSRE_M025_EVENT_CAPACITY);--count;st.queue_count=count;return true;}
const psre_m025_status_t *psre_m025_get_status(void){return &st;}
static void dec(uint32_t v){char b[10];unsigned n=0;if(!v){serial_write("0");return;}while(v&&n<sizeof(b)){b[n++]=(char)('0'+v%10u);v/=10u;}while(n)serial_write_char(b[--n]);}
void psre_m025_report(void){serial_write("\n--- M026 MULTI-ENDPOINT INPUT MANAGER ---\nInitialized=");serial_write(st.initialized?"yes":"no");serial_write(" Keyboard=");serial_write(st.keyboard_active?"active":"inactive");serial_write(" Mouse=");serial_write(st.mouse_active?"active":"inactive");serial_write(" polls=");dec(st.polls);serial_write(" key polls=");dec(st.keyboard_polls);serial_write(" mouse polls=");dec(st.mouse_polls);serial_write(" errors=");dec(st.transfer_errors);serial_write(" queued=");dec(count);serial_write(" generated=");dec(st.events_generated);serial_write(" dropped=");dec(st.events_dropped);serial_write("\n");}
