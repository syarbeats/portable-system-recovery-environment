# M005 Architecture

`storage.c` owns the four ATA channel/device slots and implements bounded polling for IDENTIFY DEVICE and READ SECTORS (PIO, one sector). `init.c` performs a read-only sector-zero scan and follows supported MBR FAT32 partition entries. `fat32.c` validates the FAT32 BIOS Parameter Block fields and reports basic volume geometry.

The storage driver uses legacy primary/secondary I/O ports (0x1F0/0x170, control 0x3F6/0x376). No PCI BAR is programmed and no disk data is modified. Polling loops have finite iteration limits to avoid an infinite wait on absent or failed hardware.

The driver is intentionally a compatibility-mode milestone, not a universal storage stack. A subsequent milestone should add PCI IDE/AHCI discovery, AHCI command lists/FIS handling, NVMe, GPT, filesystem mounting and read-only file operations.
