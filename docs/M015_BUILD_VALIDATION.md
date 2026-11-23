# M015 Build & Validation

## Static validation
- Compile all C translation units with host GCC using freestanding project flags and `-Wall -Wextra -Werror`.
- Compile the C++ subsystem independently.
- Run `make -n all`.
- Verify the integrated ZIP archive with `ZipFile.testzip()`.

## Runtime validation still required
1. Boot in QEMU with an emulated xHCI controller and capture serial output.
2. Confirm M014 discovers and validates the controller.
3. Run `m015-init` manually.
4. Verify BIOS handoff (if advertised), HCH/HCRST/CNR transitions and RUNNING state.
5. Verify DMA addresses are below 4 GiB and properly aligned.
6. Confirm no USB interrupts are enabled and no USB devices are enumerated.
7. Repeat on physical hardware only after QEMU validation.

A successful host compile is not evidence of a successful hardware initialization. Cross-compiler, NASM, ISO, QEMU and physical-controller tests must be recorded separately.
