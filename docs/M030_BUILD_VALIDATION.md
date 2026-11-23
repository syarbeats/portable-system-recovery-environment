# M030 Build Validation — Build 2.5

| Check | Result |
|---|---|
| Strict host C syntax (`-Wall -Wextra -Werror`) | PASS — 43/43 translation units |
| Freestanding C object compilation | PASS — 43/43 object files |
| C++ subsystem syntax compilation | PASS |
| C++ subsystem object compilation | PASS |
| Root Makefile dry-run | PASS |
| M030 host unit harness | PASS |
| ZIP integrity | PASS (verified after packaging) |
| Cross-toolchain kernel link | Not run; x86_64-elf toolchain unavailable |
| NASM assembly / ISO generation | Not run; toolchain and xorriso unavailable |
| QEMU boot / physical USB test | Not run |

Host unit harness covered initialization, event FIFO, all-event and filtered
subscriptions, callback counts, key state, mouse button state, relative mouse
coordinates and dequeue. It uses stubs for M025 and serial output; it does not
exercise xHCI hardware.
