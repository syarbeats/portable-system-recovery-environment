# M014 Test Plan

## Software/static tests
- Compile all C sources with project freestanding flags and `-Werror`.
- Compile C++ subsystem and NASM objects using the project's cross toolchain.
- Verify the integrated root Makefile includes `kernel/src/m014/xhci_live.c`.
- Verify the ZIP archive passes `unzip -t`.

## QEMU/hardware tests
1. Boot without an xHCI controller: M014 must report no controller and continue.
2. Boot with a QEMU xHCI controller: M014 should identify its PCI BDF and BAR.
3. Confirm valid live capability fields and `mmio_active_after_probe == 0`.
4. Run `m014-probe` repeatedly; mapping count should return to zero each time.
5. Run `m014-info`; it must report the last probe without touching hardware.

## Expected limitations
A failed mapping or parser validation is a safe failure, not evidence of a
working controller. Runtime QEMU and physical-hardware tests are required
before claiming device compatibility.
