# M011 — Architecture
## PCI selection
The driver consumes the existing M010 PCI inventory and selects the first xHCI function (class 0x0C, subclass 0x03, PI 0x30). It records the first memory BAR but does not dereference it.

## DMA memory
The PMM now supports `pmm_alloc_contiguous_pages(count, max_address)` and `pmm_free_contiguous_pages(base, count)`. M011 reserves three contiguous physical pages below 4 GiB and obtains a CPU virtual pointer using the existing Limine HHDM offset.

Layout:
- Page 0: 256 x 16-byte command TRBs; entry 255 is a Link TRB back to the ring base, with Toggle Cycle.
- Page 1: 256 x 16-byte event TRBs.
- Page 2: one ERST entry describing the event ring segment.

The software command enqueue routine in the self-test writes a No-Op Command TRB but deliberately does not ring a hardware doorbell. The event ring is prepared but not consumed by hardware.

## Safety
No MMIO access, PCI config write, controller reset, bus-master enable, or interrupt setup is performed. A future hardware bring-up milestone must add a dedicated MMIO mapper with appropriate cache policy, PCI resource validation, BIOS/OS ownership handoff, DMA coherency handling, controller halt/reset sequencing, interrupter setup and bounded timeouts before programming xHCI registers.
