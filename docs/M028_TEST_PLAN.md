# M028 Test Plan

## Static validation
- Compile every C translation unit with `-Wall -Wextra -Werror`.
- Compile C objects using the project's freestanding x86_64 flags.
- Compile the C++ subsystem.
- Dry-run the root Makefile and verify M028 is included.
- Verify archive integrity and presence of all prior milestone files.

## Hardware test sequence
1. Boot PSRE and initialize xHCI with `m015-init`.
2. Enumerate the USB device and activate its configuration (`m016`–`m020`).
3. Run `m021-probe`.
4. Run `m026-start`; confirm keyboard and dedicated mouse endpoints are active.
5. Run `m028-info` and inspect interface, endpoint and descriptor field counts.
6. Run `m026-poll` repeatedly while moving the mouse, scrolling and pressing buttons.
7. Drain events using `m025-next`; verify X/Y, buttons 1–8 and wheel events.

## Required physical test matrix
- 3-button mouse, 5-button mouse, wheel mouse, horizontal tilt-wheel mouse.
- Negative X/Y movement and negative wheel values.
- Short/undersized report rejection.
- Report-ID and no-report-ID descriptors.
- Separate keyboard + mouse interfaces concurrently.

No QEMU or physical-device runtime result is asserted by the static validation report.
