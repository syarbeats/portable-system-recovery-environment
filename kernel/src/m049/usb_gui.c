#include <psre/m049/usb_gui.h>
#include <psre/m015/xhci_init.h>
#include <psre/m016/usb_ports.h>
#include <psre/m017/usb_enum.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/m019/usb_config.h>
#include <psre/m020/usb_device.h>
#include <psre/m021/hid.h>
#include <psre/m025/input.h>
#include <psre/m030/input.h>
#include <psre/m031/framebuffer.h>
#include <psre/m034/window.h>
#include <psre/m035/shell.h>
#include <psre/serial.h>
#include <psre/m054/boot_ui.h>

#ifndef PSRE_M049_ENABLE_AUTOSTART
#define PSRE_M049_ENABLE_AUTOSTART 1
#endif
#define M049_MAX_PORTS 64u
#define M049_POLL_TIMEOUT_TICKS 1u
static psre_m049_status_t st;

bool psre_m049_initialize(void) {
    if (st.attempted) return st.initialized;
    st = (psre_m049_status_t){0};
    st.attempted = true;
    st.stage = PSRE_M049_STAGE_XHCI;
    psre_m054_boot_ui_step("USB xHCI controller initialization", "INFO");
    const psre_m031_framebuffer_t *fb = psre_m031_get();
    if (!fb || !fb->available) {
        serial_write("[M049] Framebuffer unavailable; retaining serial recovery.");
        st.stage = PSRE_M049_STAGE_FAILED; return false;
    }
#if !PSRE_M049_ENABLE_AUTOSTART
    serial_write("[M049] USB auto-enumeration disabled by safe default. Define PSRE_M049_ENABLE_AUTOSTART=1 to opt in.\n");
    st.stage = PSRE_M049_STAGE_FAILED; return false;
#else
    if (!psre_m015_initialize()) {
        serial_write("[M049] xHCI initialization failed; serial recovery retained.\n");
        st.stage = PSRE_M049_STAGE_FAILED; return false;
    }
    st.controller_ready = true;
    psre_m054_boot_ui_step("USB host controller ready", "OK");
    const psre_m015_status_t *hc = psre_m015_get_status();
    if (!hc || !hc->max_ports) { st.stage=PSRE_M049_STAGE_FAILED; return false; }
    uint8_t limit = hc->max_ports > M049_MAX_PORTS ? M049_MAX_PORTS : (uint8_t)hc->max_ports;
    bool enumerated = false;
    st.stage = PSRE_M049_STAGE_PORT;
    psre_m054_boot_ui_step("Scanning USB root ports", "INFO");
    /* Try each connected port until one yields a working HID device. A non-HID
       device (flash drive, hub) on a lower port no longer blocks the keyboard/mouse. */
    for (uint8_t port=1; port<=limit; ++port) {
        ++st.ports_examined;
        if (!psre_m016_reset_port(port)) continue;
        psre_m054_boot_ui_step("USB device detected; resetting port", "OK");
        st.stage = PSRE_M049_STAGE_SLOT;
        if (!psre_m017_enable_slot(port)) { ++st.enumeration_failures; psre_m054_boot_ui_step("Enable Slot command", "WARN"); continue; }
        psre_m054_boot_ui_step("Enable Slot command", "OK");
        st.stage = PSRE_M049_STAGE_ADDRESS;
        if (!psre_m018_enumerate(port)) { ++st.enumeration_failures; psre_m054_boot_ui_step("USB Address Device / descriptor", "WARN"); continue; }
        psre_m054_boot_ui_step("USB Address Device / descriptor", "OK");
        st.selected_port = port; st.device_enumerated = true;
        st.stage = PSRE_M049_STAGE_CONFIG;
        psre_m054_boot_ui_step("Reading USB configuration descriptors", "INFO");
        if (!psre_m019_read_configuration() || !psre_m020_activate_configuration()) {
            ++st.enumeration_failures; psre_m054_boot_ui_step("USB configuration", "WARN"); st.device_enumerated = false; continue;
        }
        st.configuration_ready=true;
        psre_m054_boot_ui_step("USB configuration activated", "OK");
        st.stage=PSRE_M049_STAGE_HID;
        psre_m054_boot_ui_step("Probing USB HID interface", "INFO");
        if (!psre_m021_probe_hid()) {
            ++st.enumeration_failures; psre_m054_boot_ui_step("USB device is not HID", "WARN");
            st.device_enumerated = false; st.configuration_ready = false; continue;
        }
        /* Register this device's keyboard/mouse endpoint(s) now, while M018/M021 still
           describe it, then keep scanning for the other device (QEMU/real PCs usually
           expose keyboard and mouse as separate USB devices). */
        st.hid_ready = true;
        if (psre_m025_add_device()) enumerated = true;
        const psre_m025_status_t *got = psre_m025_get_status();
        if (got && got->keyboard_active && got->mouse_active) break;
    }
    if (!enumerated) {
        serial_write("[M049] No USB HID device could be enumerated; continuing with graphical headless desktop.\n");
        psre_m054_boot_ui_step("USB HID enumeration unavailable", "WARN");
        st.stage=PSRE_M049_STAGE_FAILED; return false;
    }
    st.hid_ready=true;
    psre_m054_boot_ui_step("USB HID interface recognized", "OK");
    st.stage=PSRE_M049_STAGE_INPUT;
    psre_m054_boot_ui_step("Starting keyboard and mouse input stack", "INFO");
    if (!psre_m030_initialize()) {
        serial_write("[M049] No supported HID endpoint; graphical desktop fallback remains available.\n");
        psre_m054_boot_ui_step("HID input endpoint unavailable", "WARN");
        st.stage=PSRE_M049_STAGE_FAILED; return false;
    }
    const psre_m025_status_t *in=psre_m025_get_status();
    st.keyboard_ready=in && in->keyboard_active;
    st.mouse_ready=in && in->mouse_active;
    st.input_ready=true;
    psre_m054_boot_ui_step("Keyboard and mouse input router ready", "OK");
    st.stage=PSRE_M049_STAGE_GUI;
    psre_m054_boot_ui_step("Starting window manager and desktop shell", "INFO");
    if (!psre_m034_initialize() || !psre_m035_initialize()) {
        serial_write("[M049] GUI initialization failed; serial recovery retained.\n");
        st.stage=PSRE_M049_STAGE_FAILED; return false;
    }
    st.gui_ready=true; st.initialized=true; st.stage=PSRE_M049_STAGE_READY;
    psre_m054_boot_ui_step("Interactive desktop initialized", "OK");
    serial_write("[M049] USB HID input and PSRE desktop are ready.\n");
    return true;
#endif
}

bool psre_m049_poll(uint32_t timeout_ticks) {
    if (!st.initialized || !st.gui_ready || !timeout_ticks) return false;
    ++st.event_polls;
    bool activity = psre_m030_poll(timeout_ticks);
    psre_input_event_t event;
    bool dispatched=false;
    while (psre_m030_next_event(&event)) {
        /* M034 receives this event through its synchronous M030 subscription.
           Dispatch only the shell layer here to avoid duplicate WM actions. */
        (void)psre_m035_handle_event(&event);
        ++st.events_dispatched;
        dispatched=true;
    }
    return activity || dispatched;
}
const psre_m049_status_t *psre_m049_get_status(void) { return &st; }
