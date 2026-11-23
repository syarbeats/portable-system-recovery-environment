# M027 Architecture

`m027/keyboard.c` owns the 256-usage state bitmap, rollover filtering, repeat deadline, and lock LED state. `m025/input.c` forwards valid 8-byte reports from M023 to M027 and asks M027 whether a repeat event is due on each explicit M026 poll. Repeat events enter the existing 64-entry M025 FIFO. LED changes use the new bounded M018 EP0 control-OUT data transfer helper (`SET_REPORT`, Output report, one byte).

The driver remains in HID Boot Protocol. This avoids interpreting arbitrary report layouts as if they were Boot Keyboard reports; descriptor-driven report-protocol NKRO is deferred.
