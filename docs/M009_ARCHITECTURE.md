# M009 — Architecture
A dedicated 128 KiB statically allocated RAM pool stores queued payloads; a 16-entry metadata table stores path, offset, size, CRC32 and SHA-256. `queue-add PATH` invokes the M008 FAT32 recovery routine, then copies its verified in-memory staging buffer into the queue pool. The queue is independent of the single-file M008 workspace and survives subsequent `recover` commands until `queue-clear` or reboot.

Serial export uses `PSRE-MULTI-HEX-EXPORT/1`; each file is bracketed by `BEGIN` and `END` records, with 32-byte HEX data records and per-file SHA-256. A `PROGRESS:n/total` record is printed after each file.

This design intentionally does not claim USB support. USB output requires a host controller (xHCI/EHCI), enumeration, Mass Storage Class Bulk-Only Transport, SCSI READ/WRITE(10), and a block-device interface. The current project only has ATA PIO storage.
