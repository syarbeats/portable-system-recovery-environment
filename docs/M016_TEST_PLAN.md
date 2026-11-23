# M016 Test Plan

## Static/build tests
- Compile all C translation units with project-style freestanding flags and `-Wall -Wextra -Werror`.
- Verify root Makefile includes `kernel/src/m016/usb_ports.c`.
- Verify M016 is not called during automatic kernel boot.
- Verify CLI parser rejects empty, zero, non-decimal and >255 port arguments.
- Review PORTSC writes to ensure unrelated RW1C flags are never echoed.

## QEMU runtime tests
1. Configure QEMU with an xHCI controller and at least one USB device.
2. Boot PSRE and execute `m014-probe`, then `m015-init`.
3. Run `m016-ports`; compare reported port count and connection status with the QEMU device configuration.
4. Run `m016-reset N` for a connected port and inspect reset completion, speed ID, PLS and PED.
5. Try a disconnected port; reset must be rejected.
6. Try an out-of-range port; no PORTSC access/write should occur.
7. Test a SuperSpeed device if the virtual controller/device combination supports it.

## Hardware tests
- First test on a non-production machine with serial console and an alternate input path.
- Record controller VID:DID, port number, speed ID, PORTSC before/after, and timeout stage.
- Do not use USB keyboard/mouse as the only input path during controller takeover.
- No physical-controller runtime test is claimed by the source build alone.
