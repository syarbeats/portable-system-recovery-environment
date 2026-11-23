# M008 — Architecture
The M008 layer is appended to the existing FAT32 translation unit to reuse its private, bounds-checked FAT traversal and sector-read helpers. `psre_m008_recover()` resolves a path using the M007 resolver, allocates a candidate buffer using the kernel heap, reads the exact file length, then atomically replaces the previous staged buffer after a complete read.

Integrity:
- CRC32: reflected CRC-32/ISO-HDLC, polynomial `0xEDB88320`.
- SHA-256: standard SHA-256 compression and padding.

Workspace state is a single in-memory record: buffer, byte count, source path, monotonically increasing session number, CRC32 and SHA-256. `export` emits `PSRE-HEX-EXPORT/1`, `SIZE`, ordered `D:` HEX payload lines (32 bytes each), and `END:` SHA-256.

No ATA write path is present. The output channel is serial text only.
