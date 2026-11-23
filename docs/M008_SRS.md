# M008 — Software Requirements Specification
## Goal
Stage one FAT32 file into volatile kernel RAM, calculate integrity hashes, publish a manifest, and export data over the existing serial console without modifying the source disk.

## Functional requirements
1. `recover PATH` accepts a regular file addressed using M007 FAT 8.3 path syntax.
2. Reject files larger than 131072 bytes.
3. Read exactly the directory-entry file size through the FAT cluster chain; abort and release the candidate buffer on I/O or chain errors.
4. Retain one staged file in RAM. A successful new recovery replaces the prior staged file; a failed recovery preserves the prior session.
5. Calculate CRC-32/ISO-HDLC and SHA-256 over staged bytes.
6. `workspace`, `manifest`, `export`, and `clear` expose session management.
7. Source storage remains read-only.

## Non-functional requirements
- No dynamic allocation above the 128 KiB file limit.
- No ATA write calls.
- Serial export uses printable ASCII records to support terminal capture.
- No claim of host-side file creation: decoding is a host-side follow-up step.
