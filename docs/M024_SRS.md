# M024 — USB HID Mouse Driver

## Objective
Implement a first operational mouse input driver on top of the M022 Interrupt IN transfer service.

## Supported profile
- USB HID boot mouse only: Interface Subclass 1, Protocol 2.
- Sends `SET_PROTOCOL(Boot)` using class/interface request `0x21/0x0B`, value 0.
- Parses the standard 3-byte boot mouse report: button bitmap, signed X delta, signed Y delta.
- Tracks the three standard buttons and relative movement.

## API
- `psre_m024_initialize()`
- `psre_m024_process_report(report, length)`
- `psre_m024_get_status()`
- `psre_m024_report()`

## Limitations
- Wheel/extended buttons and arbitrary Report Protocol layouts are not implemented.
- No absolute pointer, acceleration, cursor rendering, or coordinate clipping.
- The active M022 endpoint must be the mouse interface selected by the caller.
