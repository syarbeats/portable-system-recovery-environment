# M007 Software Requirements — Read-only Recovery Operations

## Goal
Extend M006 with safe path-aware inspection and integrity checking without modifying source media.

## Functional requirements
- FR-01: Maintain a current working directory per mounted FAT32 volume.
- FR-02: Support `pwd`, `cd`, `ls [PATH]`.
- FR-03: Support root-relative and current-directory-relative paths.
- FR-04: Recognize `/`, `.`, and `..`.
- FR-05: Support FAT 8.3 short-name components.
- FR-06: Support `cat PATH` for files.
- FR-07: Support `stat PATH` for metadata (type, size, attributes, first cluster).
- FR-08: Support `crc32 PATH` using CRC-32/ISO-HDLC.
- FR-09: Bound directory/FAT-chain traversal by the volume's data-cluster count.
- FR-10: Report read and corrupt-chain errors without writing to media.

## Non-functional requirements
- NFR-01: Freestanding C, no dynamic allocation required for M007.
- NFR-02: Sector access must remain within the mounted volume and ATA device bounds.
- NFR-03: All operations are read-only.
- NFR-04: Preserve M001-M006 integration and one-root Makefile build.

## Out of scope
Writes, file copy to another disk, file deletion/creation, FAT repair, LFN, exFAT, NTFS, GPT, AHCI, NVMe, USB mass storage, and subdirectory mutation.
