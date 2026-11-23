# Build 4.9.3.2 — GUI Handoff & Desktop Event Loop

## Changes
- GUI readiness is determined by M048 desktop initialization, not optional USB HID auto-enumeration.
- Boot diagnostics are no longer the final framebuffer painter after GUI startup.
- After all boot diagnostics, M035 desktop is rendered again.
- When `desktop_ready` is true, the kernel enters the M050 cooperative event loop and does not start the M006 serial CLI.
- M006 remains the fallback only when desktop initialization genuinely fails.

## Expected behavior
With a valid Limine framebuffer:
1. Boot diagnostics are displayed.
2. M030 headless input is initialized if USB HID is unavailable.
3. M034 and M035 initialize.
4. Final boot log is emitted.
5. M035 desktop is repainted.
6. Kernel remains in `psre_m050_run_once()` + `hlt` loop.

Without a valid framebuffer or if GUI initialization fails, serial recovery CLI remains available.

## Validation boundary
This source patch does not prove successful cross-link, ISO build, QEMU run, or physical-PC boot. These require execution with the project's x86_64-elf toolchain and Limine release.
