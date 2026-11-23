# PSRE Integrated Build — M001 + M002 + M003

This cumulative project contains the M001 boot/serial baseline, M002 memory subsystem, and M003 CPU/interrupt subsystem in one source tree. Extract once and build from the project root; no copying between milestone archives is needed.

## Build
```sh
make clean
make
```
Output: `build/psre.elf`.

## Dependencies
- `x86_64-elf-gcc`, `x86_64-elf-g++`, `x86_64-elf-ld`
- NASM and GNU Make
- For ISO: official Limine binary release under `third_party/limine/` and `xorriso`
- For runtime validation: QEMU + OVMF

## Initialization order
1. Serial console (M001)
2. Limine memory map and HHDM (M002)
3. PMM, heap, VMM inspection, memory diagnostics (M002)
4. CPU detection, GDT, IDT, ISR, PIC, PIT (M003)
5. Enable interrupts and wait for 100 PIT ticks

## Validation boundary
This is an integrated source tree, not a claim that UEFI/ISO/QEMU runtime has passed. Build with the stated toolchain and validate in QEMU/OVMF before using physical hardware. APIC, TSS/IST double-fault stack, SMP, scheduler, and advanced routing remain future work.

## Validation performed while preparing this package
- All C translation units compiled with freestanding x86_64 flags using the available host GCC: PASS.
- C++ translation unit compiled with freestanding x86_64 flags using the available host G++: PASS.
- ZIP archive integrity: PASS.
- NASM assembly, final kernel link, ISO creation, and QEMU/OVMF boot: NOT RUN in this environment (NASM, cross toolchain, Limine binaries, and QEMU/OVMF are not available here).
