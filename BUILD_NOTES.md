# PSRE Build 2.5 — M001–M030

Cumulative recovery OS project. Build 2.5 adds M030 Kernel Input Subsystem on top of Build 2.4.

## M030 scope
- Independent 128-event FIFO above M025.
- Unified keyboard and mouse event routing.
- Keyboard HID usage bitmap and modifier state.
- Mouse button state and accumulated relative coordinates.
- Up to eight filtered or all-event synchronous subscribers.
- CLI: `m030-start`, `m030-poll`, `m030-next`, `m030-info`.

## Validation
- 43/43 C translation units: strict host syntax compile PASS.
- 43/43 C translation units: freestanding object compile PASS.
- C++ subsystem syntax/object compile PASS.
- Root Makefile dry-run PASS.
- M030 host unit harness (event routing, subscriptions, key/button state, mouse deltas, dequeue): PASS.
- QEMU boot and physical USB runtime: NOT RUN.
- Cross-toolchain link and ISO generation: NOT RUN.

The host unit test does not validate xHCI hardware behavior.


## Build 3.1 — M036

- Cumulative baseline: Build 3.0 M001–M035.
- Added `kernel/src/m036/app.c` and `kernel/include/psre/m036/app.h`.
- Added M036 host tests and documentation.
- Integrated M035 built-in applications and `m036-info` diagnostics.

## Build 3.9 — M043 (M001–M044)

- Added a cooperative recovery process manager for M008 single-file staging and M009 multi-file serial export.
- Added progress counters, pause/resume, explicit retry/cancel, and a 16-entry circular job history.
- M009 export is incremental: 32-byte HEX data records, up to 256 payload bytes per M043 tick.
- Integrated M041 Recover to RAM / Export Queue / Pause-Resume / Retry and M042 File Explorer Stage RAM.
- `m035-poll` advances one job tick; CLI provides `m043-stage`, `m043-tick`, `m043-pause`, `m043-resume`, `m043-retry`, `m043-cancel`, and `m043-info`.
- Validation: M032–M043 host suites 12/12; strict freestanding C objects 56/56; C++ object compile and Make dry-run passed. No cross-link, ISO, QEMU, or physical hardware validation is claimed.


## Build 4.3 / M048

Added boot coordinator. Host validation: M032-M048 suites pass when invoked via bash where historical script executable bits are absent. `make -n all` passes. No cross-link, ISO, QEMU or physical-PC boot validation performed.
