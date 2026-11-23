# M023 — USB HID Keyboard Driver

## Objective
Implement a first operational keyboard input driver on top of the M022 Interrupt IN transfer service.

## Supported profile
- USB HID boot keyboard only: Interface Subclass 1, Protocol 1.
- Sends `SET_PROTOCOL(Boot)` using class/interface request `0x21/0x0B`, value 0.
- Parses the standard 8-byte boot keyboard input report: modifier byte, reserved byte, six keycode slots.
- Tracks modifier state and six simultaneous key slots.

## API
- `psre_m023_initialize()`
- `psre_m023_process_report(report, length)`
- `psre_m023_get_status()`
- `psre_m023_report()`

## Limitations
- No arbitrary Report Protocol keyboard layouts, NKRO, LED/output report, keymap translation, locale handling, or key repeat.
- The driver reports HID usage keycodes; it does not convert them to ASCII.
- The active M022 endpoint must be the keyboard interface selected by the caller.
