# PSRE M004 Test Plan

1. Compile M004 C files with freestanding x86_64 flags and warnings as errors.
2. Verify the Limine RSDP request remains in `.limine_requests`.
3. Link with M001–M003 and boot QEMU + OVMF with serial output.
4. Check ACPI RSDP/root table discovery and checksum status.
5. Check PCI config mechanism #1 and compare device BDF/IDs with QEMU `info pci`.
6. Confirm no PCI configuration writes occur in M004.
7. On physical hardware, run inventory-only and save serial output before driver development.

Expected markers include `[PASS] PCI config mechanism #1 initialized`, `PCI devices discovered`, and `PSRE INTEGRATED M001-M004: INITIALIZATION COMPLETE`.

Limitations: legacy config mechanism #1 only; no ECAM, PCIe extended config space, hotplug, driver binding or DMA.

