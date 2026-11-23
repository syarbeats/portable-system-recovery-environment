# PSRE Technical Architecture

## 1. Architectural style

PSRE uses a layered, modular monolithic kernel for the early milestones. Components are separated by interfaces and directories, but execute in kernel space until process isolation and user mode are implemented.

```text
UEFI Firmware
    |
    v
Limine Bootloader / Limine Protocol
    |
    v
PSRE Kernel (x86_64, higher-half ELF)
    |-- Architecture layer (CPU, port I/O, interrupts, paging)
    |-- Kernel core (memory, scheduler, logging, synchronization)
    |-- Driver framework (PCI, USB, storage, network, display)
    |-- System services (block, filesystem, network, diagnostics)
    |-- User interface (serial/CLI first, framebuffer GUI later)
    `-- Recovery applications (disk, backup, restore, diagnostics)
```

## 2. Layer responsibilities

### Boot layer
- UEFI loads Limine from the EFI System Partition or ISO EFI boot image.
- Limine loads `psre.elf` and transfers control using the Limine protocol.
- M001 uses a minimal kernel entry and bootloader-provided execution environment.

### Architecture layer (`arch/x86_64`)
- CPU feature detection, port I/O, GDT/IDT, APIC, paging, context switching.
- Architecture-specific code must not leak into generic kernel services.

### Kernel core (`kernel`)
- Kernel entry, logging, panic handling, memory allocation, scheduler, synchronization.
- C is used for low-level core code. C++ is used for higher-level kernel subsystems with a deliberately restricted runtime.

### Drivers (`drivers` — introduced in later milestones)
- Bus enumeration, device lifecycle, DMA abstractions, interrupt registration.
- Drivers expose common interfaces to services rather than applications.

### Services (`services` — introduced in later milestones)
- Block device, filesystem, network stack, diagnostics, recovery orchestration.

### Applications (`apps` — introduced in later milestones)
- CLI utilities and later GUI applications. Destructive operations require explicit user confirmation.

## 3. M001 boot flow

1. UEFI firmware starts Limine.
2. Limine reads `limine.conf` and loads `boot/psre.elf`.
3. Limine enters the ELF entry point in x86_64 long mode.
4. `kernel_main` initializes COM1 serial output.
5. C++ subsystem function prints its component banner through the C logging API.
6. Kernel prints a successful milestone message and halts using `hlt`.

## 4. Key interfaces

- `psre::log::write(const char*)`: C++-facing logging interface backed by C serial output.
- `psre_kernel_main`: C-linkage kernel entry point.
- `arch_halt()`: architecture-specific safe halt loop.

## 5. Memory and address-space strategy

M001 relies on Limine's initial mappings and stack. M002 will request and inspect the memory map, reserve kernel/bootloader regions, and introduce a physical page-frame allocator. Paging changes will only be made after the memory map and HHDM behavior are implemented and tested against the pinned Limine protocol revision.

## 6. Console strategy

M001 uses COM1 (I/O port 0x3F8) for deterministic diagnostics in QEMU. A framebuffer console is planned after the framebuffer request and pixel-format handling are implemented. The interactive CLI follows after keyboard input and terminal support.

## 7. Boot media strategy

- ISO: Limine's ISO boot files plus `limine.conf` and `psre.elf`.
- USB: initially write the prepared hybrid boot image to a dedicated USB device; this erases the target device. A safer FAT32/EFI-folder installation workflow may be added later.
- UEFI is the primary acceptance target. BIOS boot depends on the selected Limine release's supplied BIOS image and installation procedure.

## 8. Security and safety boundaries

- No automatic disk writes or formatting.
- Recovery tools must default to read-only inspection.
- Device access must validate capacities, bounds, and alignment.
- Future destructive operations require clear target identification and confirmation.


## M015 — xHCI Controller Initialization
M015 is an explicit, user-triggered hardware transition. It performs xHCI ownership handoff, halt/reset, DMA structure allocation, operational register setup and controller start. It is intentionally not part of automatic boot initialization. USB enumeration remains out of scope.


## M016 — USB Port Detection & Reset
M016 reads xHCI root-hub PORTSC registers after M015 reaches RUNNING. It supports port inventory and an explicit port reset operation; it does not yet enumerate devices or transfer USB data.
