# M006 — Filesystem and Recovery CLI SRS

## Goal
Add a minimal, read-only FAT32 filesystem layer and interactive serial command-line interface to the integrated PSRE kernel.

## Functional requirements
1. Discover the first valid FAT32 volume on a detected ATA device, either as a superfloppy BPB or a primary MBR partition of type 0x0B/0x0C/0x1B/0x1C.
2. Validate BPB fields and bounds using the M005 sector-read interface.
3. Read FAT32 cluster chains with loop/iteration guards and partition bounds checks.
4. List the root directory, skipping deleted entries, volume labels and LFN records.
5. Print a root-level regular file selected by its 8.3 short name.
6. Provide serial CLI commands: `help`, `info`, `mount`, `ls`, `cat NAME`, and `halt`.
7. Keep all storage operations read-only.

## Constraints / non-goals
No write support, directory traversal, LFN decoding, FAT16/exFAT/NTFS, GPT, AHCI, NVMe, ATAPI, USB storage, journaling, undelete or data carving. The first mounted FAT32 volume is selected automatically.

## Acceptance criteria
- Kernel build includes M001–M006 from one Makefile.
- `make run` launches QEMU with COM1 attached to stdio.
- With a compatible FAT32 ATA image attached, boot output reports a mounted volume; `ls` lists root entries; `cat FILE.TXT` prints a root-level file.
- Without a compatible disk, the shell remains usable and reports no mounted filesystem.
- No ATA write command is issued by M005/M006.
