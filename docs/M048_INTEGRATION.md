# M048 Integration Notes

- New module: `kernel/include/psre/m048/boot.h`, `kernel/src/m048/boot.c`.
- `kernel/src/main.c` invokes M048 after M014.
- Root Makefile includes M048 and `test-m048`.
- The existing M006 serial shell remains in the boot sequence.
- The desktop starts only if M030 is already initialized; automatic HID enumeration and continuous GUI polling are not yet integrated.
