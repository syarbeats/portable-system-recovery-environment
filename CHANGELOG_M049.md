# CHANGELOG — M049

- Added guarded boot-time xHCI + one-device USB enumeration pipeline.
- Integrated M017/M018 address enumeration, M019/M020 configuration, M021 HID parsing, M025 input endpoints, and M030 routing.
- Added M034/M035 desktop initialization after input readiness.
- Added bounded `psre_m049_poll()` event pump dispatching to window manager and desktop shell.
- Added CLI commands `m049-start`, `m049-info`, and `m049-poll`.
- Added `M049_AUTOSTART` build option; safe default remains disabled.
- USB hubs, multi-device enumeration, hot-plug, scheduler/background event loop, and physical hardware validation remain future work.
