# M049 — Automatic USB HID Enumeration & GUI Event Pump

## Objective
Provide a guarded boot-time path from xHCI initialization through one USB device's enumeration, configuration, HID discovery, input-manager startup, and desktop startup. Provide a bounded polling API to dispatch input events to the window manager and desktop shell.

## Scope
- One xHCI controller and one successfully enumerated USB device per controller initialization.
- USB ports are examined in ascending order, up to 64 ports.
- HID interfaces supported by the existing M021/M023/M028/M025 stack.
- M030 routes input; M034 handles window/cursor events; M035 handles desktop interactions.
- Serial CLI remains available as recovery/fallback.

## Out of scope / limitations
- USB hubs, multi-device enumeration, hot-plug monitoring, retry/recovery after a failed Address Device, non-HID class drivers, and general USB host-controller portability.
- No background scheduler or interrupt-driven GUI loop is introduced. The kernel/CLI must call `psre_m049_poll()` regularly.
- M023 is a boot-keyboard driver; M028 is the report-protocol mouse parser. Unsupported HID layouts are rejected by existing parsers.
- xHCI takeover can affect firmware-owned USB devices. It is disabled by default.

## Build option
`make M049_AUTOSTART=1` compiles with `PSRE_M049_ENABLE_AUTOSTART=1`. Default is `0`; the `m049-start` command reports that auto-enumeration is disabled. Enabling the option is an explicit hardware-risk decision.
