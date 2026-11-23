# PSRE Build 4.9.1 — M054.1

## GUI First Boot & USB Failure Recovery

- Added framebuffer boot UI with chronological startup messages and `[ OK ]`, `[ WARN ]`, `[ FAIL ]` status markers.
- Added M030 headless input-router initialization. It allows M034/M035 desktop startup when no HID endpoint is available.
- Changed M048 boot integration to initialize graphics, input router (live or headless), window manager and desktop shell independently of USB success.
- M049 emits graphical and serial progress for xHCI, port reset, slot enable, address/descriptor, configuration, HID probe, input and GUI stages.
- USB auto-takeover defaults to OFF (`M049_AUTOSTART=0`) to protect firmware-owned USB boot paths. Explicit opt-in remains available.
- Main kernel startup now reports initialization stages to framebuffer and COM1.

## Known limitations
- The boot UI is a text-mode framebuffer status console, not a Linux kernel log implementation.
- Headless desktop displays without keyboard/mouse until a supported HID stack is initialized.
- USB enumeration still uses existing single-device M017/M018 infrastructure; multiple device enumeration is not solved by this patch.
- No QEMU runtime or physical PC validation is claimed.
