# M006 Troubleshooting — `Unable to list root directory`

Build 0.3.1 replaces the former generic `Unable to list root directory` message with filesystem-layer diagnostics.

## First checks
Run:
```text
info
mount
info
ls
```
Confirm that an ATA device is present and that the FAT32 volume is mounted.

## Diagnostics
- `volume is not mounted`: run `mount`; inspect `info` and M005 ATA detection.
- `cluster is outside the data region`: the BPB root cluster or FAT chain points outside the valid data-cluster range.
- `cluster LBA is outside volume`: BPB geometry or cluster arithmetic is inconsistent with volume bounds.
- `ATA sector read failed`: the ATA PIO read failed for the reported absolute LBA.
- `FAT sector read failed`: the FAT metadata sector could not be read.
- `invalid FAT cluster-chain entry`: the FAT entry is free, reserved, bad, or outside the data region.
- `cluster-chain traversal limit reached`: probable cyclic/corrupt chain.

## Current limitations
- ATA PIO / LBA28 only; no AHCI, NVMe, USB mass storage, or ATAPI.
- FAT32 only; MBR primary partition types 0x0B/0x0C/0x1B/0x1C or superfloppy layout.
- Root directory only; 8.3 filenames only; no LFN, subdirectory navigation, writes, or repair.
- This patch adds diagnostics and validation; it does not claim to repair damaged media.
