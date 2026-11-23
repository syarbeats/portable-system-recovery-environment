# Changelog M048

- Added non-fatal boot coordinator for framebuffer, optional xHCI, M030 input readiness, M034 window manager and M035 desktop.
- Added compile-time opt-in for xHCI controller takeover; safe default is disabled.
- Integrated coordinator into kernel boot and retained M006 serial recovery fallback.
- Documented that M017 does not yet complete Address Device, so automatic USB HID plug-and-play is not claimed.
