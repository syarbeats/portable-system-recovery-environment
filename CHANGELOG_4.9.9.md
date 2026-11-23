# Build 4.9.9 — multi-device USB HID
- M018 and M020: per-device re-enumeration/activation.
- M022: endpoints keyed by (slot, endpoint address); `psre_m022_interrupt_in_slot()`.
- M025: `psre_m025_add_device()`; keyboard_slot/mouse_slot tracked.
- M049: scans all ports, stops when keyboard and mouse are both active.
- M022: Slot Context Context Entries >= DCI on Configure Endpoint.
- Makefile: QEMU_USB default adds qemu-xhci, usb-kbd, usb-mouse.
- psre/version.h introduced as the single version source.
- Test: tests/m059 extended with two devices sharing endpoint 0x81.
