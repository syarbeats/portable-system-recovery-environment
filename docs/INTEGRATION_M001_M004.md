# PSRE Integrated M001–M004

M004 is added into the cumulative project tree. M001 boot/serial, M002 memory management, M003 interrupt/CPU, and M004 PCI/ACPI inventory all build from the same root Makefile.

## Build
```sh
make clean
make
```

## Initialization order
1. M001 serial initialization.
2. M002 memory map/HHDM, PMM, heap, VMM inspection and diagnostics.
3. M003 CPU/GDT/IDT/PIC/PIT and timer validation.
4. M004 ACPI RSDP/root table inventory.
5. M004 PCI enumeration and device inventory.

Run `docs/M004_TEST_PLAN.md` in QEMU + OVMF before physical hardware testing. Compilation alone does not prove runtime boot.

