# PSRE Integrated Build 0.4 — M001 to M006

All milestone sources are built from the root `PSRE/Makefile`. M006 is called after M005 storage discovery. `make run` builds the bootable ISO and starts QEMU with the serial terminal connected to stdio.

Use `make clean`, `make`, or `make run` from the project root. To test storage, attach a disposable FAT32 raw image using `QEMU_DISK=./test-fat32.img`. The CLI supports `help`, `info`, `mount`, `ls`, `cat NAME`, and `halt`.


## Build 0.4 M006 diagnostic update

M006 validates FAT32 data-cluster bounds and FAT capacity at mount time. Root listing now distinguishes unmounted volume, invalid cluster, FAT read failure, data-sector read failure, invalid chain entry, and traversal-limit errors. See `M006_TROUBLESHOOTING.md`.
