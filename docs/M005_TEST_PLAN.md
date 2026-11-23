# M005 Test Plan

1. Build with the documented x86_64-elf GCC/G++, binutils and NASM toolchain.
2. Boot in QEMU with an IDE disk attached; confirm ATA IDENTIFY and sector-zero read are reported.
3. Attach a disposable FAT32-formatted IDE disk image and confirm FAT32 BPB detection. Keep the image backed up; the driver itself issues read-only commands.
4. Boot without an IDE disk and confirm initialization completes with a warning rather than hanging.
5. Test an invalid/empty disk image and confirm no FAT32 volume is reported.
6. On physical hardware, verify controller compatibility before relying on results; many modern systems expose SATA through AHCI and NVMe rather than legacy IDE ports.

## Current validation boundary
A successful `make` validates compilation and linking only. `make run` requires the official Limine binary files, xorriso and QEMU. Runtime hardware detection must be validated in QEMU/physical hardware separately.
