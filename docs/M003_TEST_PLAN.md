# PSRE M003 Test Plan

## Static/build checks
1. Compile C sources with `-ffreestanding -mno-red-zone -mcmodel=kernel`.
2. Assemble ISR and GDT reload files with NASM `elf64`.
3. Link into the kernel and verify unresolved symbols are absent.
4. Inspect ELF sections and confirm `.text`/`.rodata`/`.bss` are present.

## QEMU tests
1. Boot with OVMF and a serial console.
2. Confirm CPU vendor and brand are printed.
3. Confirm IDT installation completes.
4. Confirm PIC remapping completes.
5. Enable interrupts only after initialization.
6. Confirm periodic `PIT tick` diagnostics (rate-limited).
7. In a dedicated test build, trigger `int3` and confirm vector 3 is reported.
8. In a separate test build, trigger divide-by-zero and confirm a fatal panic.

## Expected serial markers
- `[M003] CPU vendor: ...`
- `[M003] IDT installed`
- `[M003] PIC remapped: master=0x20 slave=0x28`
- `[M003] PIT configured: 100 Hz`
- `[M003] interrupts enabled`

## Caution
Do not trigger deliberate exceptions on a machine containing data you need. Test only in QEMU snapshots first.
