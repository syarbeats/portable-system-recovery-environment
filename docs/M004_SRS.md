# PSRE M004 Software Requirements Specification

## Purpose
Read-only discovery of PCI/PCIe devices and ACPI platform tables in the integrated PSRE x86_64 kernel.

## Included
- PCI Configuration Mechanism #1 through I/O ports 0xCF8/0xCFC.
- Scan buses 0–255, devices 0–31, and functions indicated by the multifunction bit.
- Capture BDF, vendor/device IDs, command/status, class/subclass/prog-if, revision, header type and legacy IRQ line/pin.
- Decode Type 0/Type 1 BARs as I/O, memory, 64-bit and prefetchable without writing them.
- Limine base revision 4 RSDP request; validate RSDP and RSDT/XSDT checksums and lengths.
- Inventory APIC/MADT, MCFG, FACP, HPET and other SDTs; count Local APIC and I/O APIC MADT entries.

## Excluded
BAR sizing/programming, PCI command changes, bus mastering, DMA, ECAM access, MSI/MSI-X, APIC setup, ACPI AML interpreter, hotplug and device drivers.

## Functional requirements
| ID | Requirement | Acceptance |
|---|---|---|
| FR-401 | Probe PCI mechanism #1 | Config read probe without config writes |
| FR-402 | Enumerate PCI | Scan B/D/F combinations |
| FR-403 | Identify device | Report BDF, vendor/device and class |
| FR-404 | Identify bridges | Count PCI-to-PCI bridge headers |
| FR-405 | Decode BAR | Read-only I/O/memory BAR decoding |
| FR-406 | Discover RSDP | Signature and checksum valid |
| FR-407 | Parse root SDT | Traverse RSDT or XSDT safely |
| FR-408 | Inventory ACPI | Count MADT/MCFG/FACP/HPET |
| FR-409 | Parse MADT | Count Local APIC and I/O APIC entries |

## Constraints
Freestanding C11, no libc or heap use, fixed-size inventory, bounded ACPI lengths, HHDM-based physical mapping. PCI configuration access is serialized against local interrupts; M004 performs no device configuration writes.

