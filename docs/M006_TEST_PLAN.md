# M006 Test Plan — Build 0.3.1

## Build checks
1. Run `make clean && make` with the documented x86_64-elf toolchain.
2. Run `make run` with QEMU and Limine assets installed.
3. Compile-check M006 C sources with warnings treated as errors.

## QEMU disk tests
Prepare a disposable raw IDE disk image containing a FAT32 primary partition. Attach it with:
`make run QEMU_DISK=./test-fat32.img`

Expected checks:
- ATA disk is detected and M006 reports a mounted FAT32 volume.
- `info` reports device, partition LBA, total sectors, sectors/cluster, root cluster, FAT count and sectors/FAT.
- `ls` lists root directory entries.
- `cat README.TXT` prints a root-level 8.3 test file.
- `mount` re-runs discovery and mount selection.

## Root-directory failure diagnostics
- No mounted volume: `ls` reports that the volume is not mounted.
- Root cluster outside the data region: mount is rejected with the invalid root cluster diagnostic.
- Invalid cluster chain: `ls` reports the current cluster.
- FAT read failure: `ls` reports the absolute LBA of the FAT sector.
- Data read failure: `ls` reports the absolute LBA of the failed data sector.
- Traversal limit: `ls` reports a possible cyclic or corrupt cluster chain.

## Negative tests
- Boot with no disk: shell still starts; `info` reports no mounted filesystem.
- Attach a non-FAT32 disk: no volume is mounted.
- Try a missing filename: `cat` reports not found.
- Nested paths and long filenames remain unsupported; use root-level 8.3 names.

## Safety
M005/M006 use ATA IDENTIFY and READ SECTORS only. No WRITE SECTORS command is implemented. Keep test images disposable.
