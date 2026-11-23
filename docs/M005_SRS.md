# M005 — Storage Subsystem (Build 0.2)

## Objective
Provide a minimal, read-only storage discovery foundation for the PSRE recovery environment.

## Scope
- Probe the four legacy ATA primary/secondary master/slave positions using IDENTIFY DEVICE.
- Record model string, reported sector count and basic capability words.
- Read one 512-byte sector using ATA PIO LBA28.
- Inspect sector zero as a FAT32 BPB or as an MBR and probe FAT32 partition types 0x0B/0x0C/0x1B/0x1C.
- Report discovered ATA disks and validated FAT32 volume metadata over COM1 serial.

## Constraints
- Read-only. No ATA write commands are implemented.
- ATA PIO legacy compatibility mode only. AHCI, NVMe, ATAPI, GPT, USB mass storage, DMA, and LBA48 reads are not implemented.
- Sector reads are limited to LBA28 (first 128 GiB with 512-byte sectors).
- FAT32 probing validates the BPB; it does not yet mount the filesystem, traverse directories, or read file contents.
