# M007 Test Plan

Prepare a disposable raw IDE disk with a FAT32 filesystem containing a root directory, a subdirectory named `SYSTEM`, and test files using 8.3 names.

## Positive tests
- `mount`, `pwd` -> mounted volume and `/`.
- `ls` -> root entries.
- `cd SYSTEM`, `pwd` -> `/SYSTEM`.
- `ls` -> entries in SYSTEM.
- `cat CONFIG.SYS` -> file content.
- `stat CONFIG.SYS` -> type, size, attributes, first cluster.
- `crc32 CONFIG.SYS` -> eight-digit CRC32.
- `cd ..`, `pwd` -> `/`.
- `ls /SYSTEM` -> subdirectory listing from root.
- `cat /SYSTEM/CONFIG.SYS` -> path-aware file read.
- `cd /` -> root.

## Negative tests
- `cd MISSING` -> directory-not-found.
- `cat MISSING.TXT` -> file-not-found.
- `stat MISSING.TXT` -> entry-not-found.
- `crc32 SYSTEM` -> reject directory target.
- Corrupt cluster pointer -> bounded error, no hang.
- Unmounted filesystem -> explicit failure.

## Integrity oracle
For CRC32, compare the displayed value against:
`python3 -c 'import zlib; print(f"{zlib.crc32(open("CONFIG.SYS","rb").read()):08X}")'`
using the same file bytes.

## Safety
No write operation is included. Confirm source code contains no ATA WRITE SECTORS command in M005/M006/M007.
