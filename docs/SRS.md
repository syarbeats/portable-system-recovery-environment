# PSRE — Software Requirements Specification

**Version:** 0.1.0 (Milestone 001 baseline)  
**Status:** Initial project baseline  
**Platform:** Custom x86_64 operating system

## 1. Purpose

PSRE (Portable System Recovery Environment) is a bootable, standalone recovery and maintenance operating system. It is intended to start independently of an installed operating system and eventually provide storage, filesystem, hardware diagnostic, network, backup, and recovery utilities.

## 2. Scope

The initial target is x86_64 PCs and QEMU. The boot media targets are a bootable ISO image and USB media. UEFI is the primary firmware interface. The selected bootloader is Limine, using its Limine boot protocol. BIOS compatibility may be provided by the Limine media image where supported by the chosen Limine release; UEFI is the guaranteed project target.

PSRE is not intended to run Windows applications or replace a general-purpose desktop operating system.

## 3. Project decisions

| Item | Decision |
|---|---|
| Name | PSRE |
| Kernel | Custom kernel |
| Architecture | x86_64 |
| Firmware | UEFI first |
| Bootloader | Limine |
| Media | ISO + USB |
| Kernel language | C (freestanding) |
| Subsystems | C++ (freestanding subset initially) |
| Interfaces | CLI first; GUI introduced in a later milestone |
| Build | GNU Make |
| First test target | QEMU x86_64 |

## 4. Stakeholders and users

- PC technicians performing diagnostics and recovery.
- System administrators maintaining PCs and storage devices.
- Developers extending PSRE drivers, services, and utilities.

## 5. Product perspective

PSRE is a layered operating system:

1. Firmware and bootloader handoff.
2. Kernel entry, CPU state, memory management, interrupt handling.
3. Hardware abstraction and device drivers.
4. Core services (storage, filesystem, network, diagnostics).
5. CLI and later graphical shell.
6. Recovery applications.

## 6. Functional requirements

| ID | Requirement | Target milestone |
|---|---|---|
| FR-001 | Boot as an x86_64 ELF kernel through Limine | M001 |
| FR-002 | Initialize a minimal kernel runtime and halt safely | M001 |
| FR-003 | Provide a serial diagnostic output channel for development | M001 |
| FR-004 | Detect and report firmware/bootloader-provided memory map | M002 |
| FR-005 | Manage physical and virtual memory | M002–M003 |
| FR-006 | Initialize CPU exception and interrupt handling | M003 |
| FR-007 | Enumerate PCI/PCIe devices | M004 |
| FR-008 | Support USB host controllers and USB storage | M005+ |
| FR-009 | Support block devices including SATA/AHCI and NVMe | M006+ |
| FR-010 | Read FAT32 and ISO9660 filesystems | M007+ |
| FR-011 | Provide file operations and a command shell | M008+ |
| FR-012 | Provide network diagnostics and Ethernet support | M009+ |
| FR-013 | Provide backup/restore and recovery utilities | M010+ |
| FR-014 | Provide a graphical desktop and application launcher | M011+ |
| FR-015 | Produce bootable ISO and USB deployment media | M001 packaging; refined later |

## 7. Non-functional requirements

- **NFR-001 Portability:** target standard x86_64 UEFI PCs; QEMU is the reference test platform.
- **NFR-002 Safety:** destructive disk operations must require explicit confirmation and must never run automatically at boot.
- **NFR-003 Reliability:** kernel failures must fail closed (halt) rather than continue in an undefined state.
- **NFR-004 Observability:** early boot must provide serial diagnostics; later releases add on-screen logs.
- **NFR-005 Maintainability:** separate architecture code, kernel core, drivers, services, and applications.
- **NFR-006 Security:** use least privilege when a user-mode model is introduced; validate all device and filesystem inputs.
- **NFR-007 Resource use:** remain usable on modest x86_64 systems; concrete RAM/CPU minimums will be established after driver scope is defined.

## 8. M001 acceptance criteria

1. `make` produces `build/psre.elf` with an x86_64 ELF format.
2. The kernel is linked at the higher-half virtual address expected by the selected Limine protocol/toolchain configuration.
3. Limine displays a PSRE boot entry from the generated media.
4. QEMU UEFI boots the kernel without resetting or triple-faulting.
5. The kernel writes the M001 startup banner and status lines to COM1 serial output.
6. The kernel reaches a controlled halt loop.

## 9. Out of scope for M001

- Physical memory allocator, paging implementation, scheduler, user mode.
- Disk, USB, NVMe, network, audio, Wi-Fi, and graphics drivers.
- Filesystem write support or destructive disk tools.
- GUI, shell command interpreter, Windows registry repair, and Windows-specific recovery logic.

## 10. Assumptions and constraints

- Limine is an external bootloader dependency and must be obtained from its official upstream release.
- The first milestone uses the bootloader-provided CPU mode and stack; it does not yet install its own GDT, IDT, page tables, or scheduler.
- C++ is compiled in freestanding mode. Exceptions, RTTI, dynamic allocation, static constructors, and standard library facilities are disabled/not used until their runtime support is implemented.
- The ISO/USB packaging commands depend on host tools such as `xorriso`, `mtools`, and a Limine binary release.

## 11. Risks

- Firmware and hardware behavior varies; validate QEMU before physical hardware.
- Bootloader protocol and media packaging evolve; pin and record the Limine release used for each reproducible build.
- Hardware driver development is substantial, especially Wi-Fi, NVMe, and modern graphics.
- NTFS support is a later project decision; initial filesystem targets are FAT32 and ISO9660.
