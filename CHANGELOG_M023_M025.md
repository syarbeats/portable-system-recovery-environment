# PSRE Build 2.0 — M023, M024, M025 Release Notes

## M023
- HID boot keyboard interface selection and SET_PROTOCOL(Boot).
- 8-byte boot keyboard report decoding.

## M024
- HID boot mouse interface selection and SET_PROTOCOL(Boot).
- 3-byte boot mouse report decoding with signed X/Y movement and three buttons.

## M025
- Shared input manager, keyboard-first/mouse-fallback selection.
- M022 endpoint setup and explicit one-report polling.
- 64-entry FIFO for key and mouse events.
- Queue overflow and transfer error counters.

## Validation and limitations
- Host strict source compilation is not hardware validation.
- One HID endpoint at a time due to current M022 lifecycle limitation.
- Boot protocol only; no arbitrary HID report layouts, keyboard-to-ASCII mapping, wheel, GUI cursor, or background polling.
- QEMU and physical xHCI device tests have not been performed.
