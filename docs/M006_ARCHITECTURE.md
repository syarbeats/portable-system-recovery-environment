# M006 Architecture

## Components
- `kernel/src/m006/fat32_fs.c`: mount discovery, BPB-derived volume geometry, FAT entry lookup, cluster traversal, root directory enumeration, and bounded root-file reads.
- `kernel/src/m006/cli.c`: blocking COM1 shell and command dispatch.
- `kernel/src/serial.c`: COM1 character input and line editing support.
- `kernel/include/psre/m006/fs.h`: public M006 interface.

## Data flow
`ATA PIO (M005) -> sector read -> FAT32 volume mount -> FAT chain / root directory -> CLI output`.

## Safety properties
- No disk writes are performed.
- Sector reads are constrained by the mounted volume's declared sector count and the ATA device's LBA28 capacity.
- Cluster-to-LBA calculations use 64-bit intermediate arithmetic and reject out-of-volume ranges.
- FAT and directory traversals have iteration guards.
- Directory/file support is intentionally limited to the root directory and short 8.3 names.

## Known limitations
The filesystem layer is an early read-only implementation, not a production recovery engine. It does not parse GPT, LFN entries, subdirectories, or filesystem corruption/recovery metadata.
