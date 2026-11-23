# M055 HID Test Plan — Updated for Build 4.9.4.1

1. Verify desktop appears with USB auto-start disabled and enabled.
2. With a boot-protocol mouse interface on the selected device, verify M024 selection, M022 endpoint configuration, successful interrupt-IN completions and M025 mouse report count.
3. With a boot keyboard, verify M023 selection, M022 endpoint configuration, successful interrupt-IN completions and M025 keyboard report count.
4. Confirm M030 event counters increase after physical key presses and mouse movements.
5. Confirm M034/M035 receive events and desktop remains visible if a transfer fails.
6. Test independent keyboard and mouse devices. M049 currently retains only the first enumerated device, so this scenario is not expected to pass until per-slot state exists.
