# PSRE — Portable System Recovery Environment

Integrated Build 4.9.15 — M001–M058

PSRE is a freestanding, x86_64 recovery operating environment that provides
read-only storage access, FAT32 filesystem recovery tools, USB and HID support,
and a graphical desktop for diagnosing and recovering data from storage devices.
It is designed to run as a bootable ISO in UEFI or BIOS environments and can be
tested directly in QEMU.

<img width="1026" height="829" alt="image" src="https://github.com/user-attachments/assets/f0c98c52-a6cd-4e29-ba1e-8fc17312fa72" />


- x86_64, UEFI/Limine boot
- C kernel core and C++ subsystem foundation
- Framebuffer desktop with serial fallback
- Cumulative recovery, storage, USB/xHCI, HID, GUI, and diagnostics milestones M001–M058

## Build

Requires `x86_64-elf-gcc`, `x86_64-elf-g++`, GNU binutils, NASM, Limine binaries, and xorriso.

```sh
make
make iso
make run
make test-m058
```

## Creating a Virtual Disk

PSRE supports IDE ATA PIO disks containing a FAT32 volume. Use a raw disk image
because the Makefile attaches the disk with `format=raw,if=ide`. The volume can
be placed directly at LBA 0 (superfloppy) or within an MBR FAT32 partition.

The following example creates a FAT32 virtual disk directly at LBA 0 on Linux:

```sh
mkdir -p build
truncate -s 64M build/psre-disk.img
mkfs.fat -F32 -n PSRE build/psre-disk.img
```

To add a test file using the 8.3 filename convention without mounting the disk,
install `mtools` and run:

```sh
printf 'Hello from PSRE\n' >/tmp/README.TXT
mcopy -i build/psre-disk.img /tmp/README.TXT ::/README.TXT
```

Alternatively, create a virtual disk with an MBR partition:

```sh
truncate -s 64M build/psre-disk.img
parted -s build/psre-disk.img mklabel msdos mkpart primary fat32 1MiB 100%
sudo losetup --find --partscan --show build/psre-disk.img
```

Record the loop-device path printed by the command, for example `/dev/loop0`.
Format the first partition (`/dev/loop0p1`) with
`mkfs.fat -F32 -n PSRE /dev/loop0p1`, add any required files, and detach the
loop device:

```sh
sudo mkfs.fat -F32 -n PSRE /dev/loop0p1
sudo mount /dev/loop0p1 /mnt
sudo sh -c "printf 'Hello from PSRE\n' >/mnt/README.TXT"
sudo umount /mnt
sudo losetup -d /dev/loop0
```

## Attaching a Virtual Disk While Running in QEMU

Build the ISO and attach the disk using the `QEMU_DISK` variable:

```sh
make run QEMU_DISK=./build/psre-disk.img
```

The Makefile passes the following disk option to QEMU:

```text
-drive file=./build/psre-disk.img,format=raw,if=ide
```

Additional QEMU options can be provided through `QEMU_EXTRA`, for example:

```sh
make run QEMU_DISK=./build/psre-disk.img QEMU_EXTRA="-rtc base=utc"
```

After QEMU boots and the `PSRE:/# ` serial prompt appears, inspect and mount the
disk with the following commands:

```text
info
mount
ls
cat README.TXT
```

PSRE also detects FAT32 volumes during boot through storage initialization. In
the graphical desktop, mount a volume using
**Recovery Manager -> Mount Volume**, then browse it through File Explorer.

Compatibility notes:

- Ensure the image is in raw format; QCOW2 images are not supported by
  `QEMU_DISK`.
- Use FAT32 with 512-byte sectors.
- Filename support is primarily limited to the 8.3 format, such as `README.TXT`.
- ATA PIO/IDE virtual disks are supported. AHCI and NVMe are not currently
  supported.
- USB Mass Storage can also provide a FAT32 volume through the `usb-mount`
  command. This is separate from the IDE disk attached by `QEMU_DISK`.

See the [M006 test plan](docs/M006_TEST_PLAN.md) for mounting behavior and
[M007 troubleshooting](docs/M007_TROUBLESHOOTING.md) if a volume cannot be found
or read.

## M058
M058 adds a read-only xHCI/HID endpoint audit. It preserves the established Desktop startup and event-loop path. A successful desktop boot does not imply keyboard/mouse input is working; confirm Transfer Event completions and M030 input counters on the target machine.

See `docs/INTEGRATION_M001_M058.md` and `docs/M058_*` for scope and validation limits.
