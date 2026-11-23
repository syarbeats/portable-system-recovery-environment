#include <psre/m048/boot.h>
#include <psre/m031/framebuffer.h>
#include <psre/m032/graphics.h>
#include <psre/m033/font.h>
#include <psre/m030/input.h>
#include <psre/m034/window.h>
#include <psre/m035/shell.h>
#include <psre/m054/boot_ui.h>
#include <psre/serial.h>
static psre_m048_status_t st;
bool psre_m048_initialize(void) {
    if (st.initialized) return st.desktop_ready;
    st=(psre_m048_status_t){0};
    const psre_m031_framebuffer_t *fb=psre_m031_get();
    if(!fb||!fb->available){st.failure_stage=1;st.initialized=true;serial_write("[M048] Framebuffer unavailable; serial recovery only.\n");return false;}
    st.framebuffer_ready=true;
    if(!psre_m032_initialize()||!psre_m033_initialize()){st.failure_stage=2;st.initialized=true;return false;}
    const psre_m030_status_t *input=psre_m030_get_status();
    if(!input||!input->initialized){
        if(!psre_m030_initialize_headless()){st.failure_stage=3;st.initialized=true;return false;}
    }
    st.input_ready=true;
    if(!psre_m034_initialize()){st.failure_stage=4;st.initialized=true;serial_write("[M048] Window manager initialization failed.\n");return false;}
    st.window_manager_ready=true;
    if(!psre_m035_initialize()){st.failure_stage=5;st.initialized=true;serial_write("[M048] Desktop shell initialization failed.\n");return false;}
    st.desktop_ready=true;st.initialized=true;
    serial_write("[M048] Desktop shell initialized (HID or headless mode).\n");
    return true;
}
const psre_m048_status_t *psre_m048_get_status(void){return &st;}
