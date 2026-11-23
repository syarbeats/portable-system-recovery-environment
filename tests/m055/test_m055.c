#include <assert.h>
#include <psre/m055/input_service.h>
#include <psre/m049/usb_gui.h>
#include <psre/m025/input.h>
static psre_m049_status_t usb;
static psre_m025_status_t input;
static unsigned polls;
const psre_m049_status_t *psre_m049_get_status(void){return &usb;}
const psre_m025_status_t *psre_m025_get_status(void){return &input;}
bool psre_m049_poll(uint32_t t){assert(t);++polls;return true;}
void serial_write(const char *s){(void)s;}
void serial_write_char(char c){(void)c;}
int main(void){
    assert(psre_m055_initialize());
    assert(psre_m055_poll(1u)==false);
    assert(polls==0u);
    usb.initialized=true;usb.gui_ready=true;input.keyboard_active=true;
    assert(psre_m055_poll(1u)==true);
    assert(polls==1u);
    const psre_m055_status_t *s=psre_m055_get_status();
    assert(s&&s->initialized&&s->usb_input_available&&s->keyboard_active);
    assert(s->poll_cycles==2u&&s->active_poll_cycles==1u&&s->idle_cycles==1u);
    return 0;
}
