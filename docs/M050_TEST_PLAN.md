# M050 Test Plan

1. Compile M050 event loop and device manager with host GCC, `-Wall -Wextra -Werror -ffreestanding`.
2. Compile touched serial, CLI and kernel entry sources with the same strict flags.
3. Verify CLI calls `psre_m050_run_once()` while UART has no input.
4. Verify kernel entry initializes the device manager and scheduler after M049.
5. Verify Makefile includes both M050 source files and `test-m050`.
6. Run all available host suites M032–M050.
7. Run C/C++ syntax checks and Make dry-run.
8. Follow with QEMU timer/serial test and physical xHCI/HID test; these are not substituted by host syntax tests.
