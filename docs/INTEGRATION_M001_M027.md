# PSRE Integrated Build 2.2 — M001–M027

M027 is cumulative on Build 2.1 (M001–M026). It adds `kernel/include/psre/m027/keyboard.h`, `kernel/src/m027/keyboard.c`, and the M018 control-OUT data request helper required for HID SET_REPORT. M025 input processing now forwards keyboard reports to M027 and emits `KEY_REPEAT` through the existing FIFO. CLI adds `m027-info`.

Build remains explicit/polled: run the existing xHCI and USB enumeration/configuration sequence, then `m026-start`; use `m026-poll` to poll endpoints and `m025-next` to retrieve events. `m027-info` reports keyboard extension state.

The current implementation remains Boot Protocol (6KRO). NKRO/report-protocol parsing is not implemented in this milestone. PIT repeat constants assume a 100 Hz PIT configuration.
