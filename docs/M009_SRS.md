# M009 — Software Requirements Specification
## Objective
Extend M008 with a bounded multi-file recovery queue and serial batch export. The source ATA/FAT32 volume remains read-only.

## Functional requirements
- Queue up to 16 regular files addressed by FAT 8.3 paths.
- Aggregate payload limit: 128 KiB in a dedicated static RAM pool.
- Each file is staged through the M008 FAT32 read path before being appended.
- Record path, byte offset, size, CRC32 and SHA-256 for every item.
- List queue, print manifest, export all files, and clear queue.
- Failed reads or capacity checks must not add an item to the queue.

## Explicit exclusions
USB Mass Storage output is not implemented in this milestone. Existing hardware support is ATA PIO only; no USB host-controller driver, USB enumeration, BOT/SCSI transport or USB block-device abstraction exists yet. This is documented as a dependency for M010.
