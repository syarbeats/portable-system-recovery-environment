# M030 Test Plan

## Static/build checks
- Compile `kernel/src/m030/input.c` with `-std=gnu11 -Wall -Wextra -Werror`.
- Compile all C units with the project freestanding flags.
- Compile the C++ subsystem and run root `make -n`.
- Verify the M030 source is present in the root `C_SOURCES` list.

## Runtime scenarios (QEMU/physical USB required)
1. `m030-start` before M025 initialization must fail safely.
2. Initialize USB and HID, then `m030-start` must succeed.
3. Poll keyboard down/up and verify M030 queue and key bitmap.
4. Poll mouse movement and verify relative coordinate accumulation.
5. Press/release mouse buttons and verify button state.
6. Register one all-events subscriber and one filtered subscriber; verify
   only matching callbacks run.
7. Unsubscribe and verify no further callback calls for that subscription.
8. Fill the M030 FIFO without dequeuing; verify drop counter increments while
   subscriber dispatch continues.
9. Verify queue wraparound and mouse coordinate saturation.

## Validation boundary
Synthetic/static tests do not establish xHCI hardware correctness. Full runtime
validation requires the PSRE cross toolchain, QEMU with xHCI USB devices, and
physical keyboard/mouse testing.
