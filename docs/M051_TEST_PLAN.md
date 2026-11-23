# M051 Test Plan

1. Compile M051 and M016 with `-std=c11 -Wall -Wextra -Werror -ffreestanding`.
2. Compile main, CLI and M050 scheduler integration.
3. Verify M050 references the periodic lifecycle scan.
4. Verify Makefile exposes `test-m051`.
5. Hardware follow-up: attach/detach a USB device while observing PORTSC CCS/CSC and event counters. Confirm M051 does not write PORTSC or issue Enable Slot/Disable Slot commands.

Host syntax checks do not emulate xHCI hardware and cannot prove physical hot-plug detection.
