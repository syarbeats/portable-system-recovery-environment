# M011 — xHCI Driver Foundation & DMA Ring Structures
## Objective
Introduce safe xHCI driver foundations into PSRE: contiguous DMA page allocation, TRB data structures, command/event ring layout, Event Ring Segment Table (ERST), controller selection, and software-only ring validation.

## Functional requirements
1. Select a PCI USB xHCI controller (class 0x0C, subclass 0x03, programming interface 0x30).
2. Record PCI BDF, vendor/device IDs, and a memory BAR from M010 inventory.
3. Allocate three physically contiguous pages below 4 GiB using the PMM.
4. Access allocated RAM through the Limine HHDM alias.
5. Build a 256-entry command ring with a Link TRB, a 256-entry event ring, and a one-entry ERST.
6. Provide `xhci-info`, `xhci-test`, and `xhci-release`.
7. Release the DMA pages using the matching contiguous PMM free operation.

## Explicit scope boundary
This build does not map or access the controller BAR, change PCI command bits, enable bus mastering, reset/start xHCI, program CRCR/ERSTBA/ERDP/doorbells, enable interrupts, enumerate ports/devices, or issue USB transfers. These operations require validated MMIO mapping/cache attributes, controller ownership handoff, DMA coherency and hardware-specific sequencing. The ring self-test is software-only and does not prove controller interoperability.
