# Changelog — M027

## Build 2.2
- Added 256-usage keyboard state bitmap.
- Added HID ErrorRollOver report rejection.
- Added key repeat (500 ms initial delay, 50 ms repeat interval at 100 Hz PIT).
- Added Caps Lock, Num Lock, and Scroll Lock LED state and HID SET_REPORT output transfer.
- Added M018 EP0 control-OUT data stage support.
- Added `KEY_REPEAT` to M025 input events and `m027-info` diagnostics.
- Retained M026 independent keyboard/mouse endpoint polling.

Limitation: M023 still selects HID Boot Protocol, so keyboard reports remain 6KRO. NKRO/report-protocol decoding is not included.
