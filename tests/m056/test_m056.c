#include <assert.h>
#include <psre/m056/diagnostics.h>
#include <psre/m031/framebuffer.h>
#include <psre/m021/hid.h>
#include <psre/m022/interrupt.h>
#include <psre/m025/input.h>
#include <psre/m030/input.h>
#include <psre/m049/usb_gui.h>
static psre_m031_framebuffer_t fb;static psre_m021_status_t hid;static psre_m022_status_t xfer;
static psre_m025_status_t input;static psre_m030_status_t router;static psre_m049_status_t usb;
const psre_m031_framebuffer_t *psre_m031_get(void){return &fb;}
const psre_m021_status_t *psre_m021_get_status(void){return &hid;}
const psre_m022_status_t *psre_m022_get_status(void){return &xfer;}
const psre_m025_status_t *psre_m025_get_status(void){return &input;}
const psre_m030_status_t *psre_m030_get_status(void){return &router;}
const psre_m049_status_t *psre_m049_get_status(void){return &usb;}
void serial_write(const char *s){(void)s;}void serial_write_char(char c){(void)c;}
int main(void){fb.available=true;hid.success=true;hid.hid_interface_count=1;hid.keyboard_count=1;usb.controller_ready=true;usb.device_enumerated=true;usb.gui_ready=true;input.keyboard_active=true;router.initialized=true;xfer.transfer_attempts=4;xfer.transfer_count=2;xfer.timeout_count=1;assert(psre_m056_initialize());const psre_m056_status_t*s=psre_m056_get_status();assert(s&&s->initialized&&s->framebuffer_ready&&s->desktop_ready);assert(s->keyboard_active&&!s->mouse_active);assert(s->transfer_attempts==4&&s->transfer_successes==2&&s->transfer_timeouts==1);return 0;}
