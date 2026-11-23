# M017 Test Plan

## Static/host checks
- Compile all C translation units with GCC `-std=gnu11 -ffreestanding -Wall -Wextra -Werror`.
- Compile C++ subsystem with the root Makefile flags.
- Run `make -n all` to validate source/object integration.
- Verify ZIP integrity and confirm prior M001-M016 files remain present.

## Hardware/QEMU tests (not performed by host compilation)
1. Boot with serial console and verify M014 xHCI discovery.
2. Execute `m015-init`; verify controller RUNNING.
3. Execute `m016-ports`; connect a supported USB device.
4. Execute `m016-reset PORT`; confirm PED=1.
5. Execute `m017-enable-slot PORT`; require Command Completion Event type 33, Completion Code 1, and non-zero Slot ID.
6. Repeat with disconnected/invalid port and confirm rejection before command submission.
7. Test unsupported controllers and timeout paths.

Do not infer successful hardware enumeration from static compilation. Preserve serial logs for each runtime test.
