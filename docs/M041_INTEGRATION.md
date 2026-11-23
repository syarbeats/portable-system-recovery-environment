# M041 — Integration

- Added `psre/m041/recovery_manager.h` and `kernel/src/m041/recovery_manager.c`.
- M035 now initializes M041 when launching Recovery Manager.
- Added the M041 host test and root Makefile target `test-m041`.
- Retained all prior source and documentation files.

GUI operations call the existing M006 filesystem/recovery API. Detailed output
is sent to serial; the window shows concise success/failure feedback.
