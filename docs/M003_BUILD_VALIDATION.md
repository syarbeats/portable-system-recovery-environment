# M003 Build and Validation Notes

## Required tools
- x86_64-elf GCC (or compatible freestanding compiler)
- NASM
- GNU Make
- Existing PSRE M001/M002 serial implementation and linker integration
- QEMU + OVMF for runtime tests

## Integration notes
- Add `arch/x86_64/m003/gdt_reload.asm` and `arch/x86_64/m003/interrupt_control.asm` to assembly build.
- Add `arch/x86_64/m003/isr.asm` to assembly build.
- Add `arch/x86_64/m003/gdt.c` and `kernel/src/m003/*.c` to C build.
- Call `psre_gdt_initialize()` early, before loading IDT, if the current boot environment does not already establish compatible kernel descriptors.
- Call `psre_m003_init()` after serial initialization.
- The GDT reload is optional if Limine-provided execution environment is intentionally retained; do not reload it without validating the selector/layout contract.
- Export the symbol `psre_isr_stub_table` from the NASM object.

## Important ABI note
The assembly common stub passes a pointer to the register frame using the System V AMD64 ABI. The C compiler must use the normal kernel ABI and `-mno-red-zone`. Stack alignment must be verified in the final linker/runtime build. For hardened production code, use a dedicated assembly entry wrapper that aligns RSP before calling C.

## Current implementation boundary
This package intentionally provides only vectors 0–47. Other IDT entries remain not-present and will cause a CPU fault if invoked. Add a default handler before enabling unexpected vectors in later revisions.
