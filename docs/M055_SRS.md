# M055 — USB HID Input Polling & Desktop Integration

## Goal
Provide a bounded periodic HID input service while keeping framebuffer/Desktop boot independent of USB availability.

## Implemented
- M055 service initializes even when USB HID is absent.
- M050 schedules M055 polling once per PIT tick.
- M055 delegates bounded input polling to M049/M030/M025.
- M049 no longer dispatches each input event twice to M034; M034 receives events through its M030 subscription, while M035 receives the shell event dispatch.
- M025 now falls back to the M024 USB boot-mouse parser when the M028 report-descriptor mouse parser does not find a mouse interface.
- Keyboard and mouse polling counters/status are exposed through `psre_m055_report()`.

## Safety invariant
Framebuffer M031, M048 desktop initialization, M035 shell rendering and the serial recovery fallback do not depend on M055 or successful HID enumeration. USB failures leave the desktop in headless mode.

## Current limitation
M049 still enumerates only the first successfully addressed USB device and exits its root-port scan. M018/M019/M021 also expose singleton current-device state. Therefore M055 supports polling keyboard and/or mouse interfaces exposed by the currently selected USB device, but does not yet claim simultaneous support for two separate USB devices (for example, one independent keyboard plus one independent mouse). That requires a later per-slot/per-device refactor.

## Runtime diagnostics
`psre_m055_report()` reports USB input availability, keyboard/mouse endpoint activation, poll cycles, idle cycles and errors.
