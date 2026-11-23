# M054.1 — GUI First Boot & USB Failure Recovery

## Objective
Display a visible graphical startup log on the Limine framebuffer and ensure USB HID failure does not prevent PSRE Desktop initialization.

## Startup behavior
1. Kernel entry and serial initialization.
2. Limine memory map/HHDM acquisition.
3. Framebuffer discovery.
4. Framebuffer boot log initialization.
5. PMM, heap, VMM, CPU tables, interrupt controller and PIT.
6. ACPI, PCI and xHCI capability inventory.
7. Optional USB HID bring-up (disabled by default).
8. M048 initializes M030 in live or headless mode.
9. M034 Window Manager and M035 Desktop Shell initialize.
10. M050 cooperative event loop and storage discovery.
11. M006 serial recovery console remains available.

## Graphical boot log
Each stage is mirrored to COM1 and framebuffer. The framebuffer log uses a rolling list with `[ OK ]`, `[ WARN ]`, and `[ FAIL ]` labels. On successful desktop initialization, M035 renders the normal desktop over the boot log.

## Headless input mode
M030 gains `psre_m030_initialize_headless()`. In this mode subscriptions and event routing remain available, but polling does not call M025. This permits M034/M035 initialization without inventing a keyboard or mouse device.

## USB safety
`M049_AUTOSTART` defaults to 0. To explicitly test xHCI takeover and HID enumeration, build with `make M049_AUTOSTART=1`. USB takeover may change firmware controller ownership and should be tested carefully on the target PC.

## Acceptance criteria
- If a valid framebuffer is provided, boot stages are drawn to it.
- If USB HID is absent or fails, PSRE attempts to display desktop in headless mode.
- Serial COM1 remains available for diagnostics and recovery.
- If framebuffer is absent, serial-only recovery remains the fallback.

## Validation status
Source-level syntax and host tests can validate APIs and parsers but cannot prove a physical display works. QEMU and physical PC boots remain required.
