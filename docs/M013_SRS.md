# M013 — PCI Configuration Write & MMIO Mapping Infrastructure

## Objective
Introduce controlled PCI configuration-space writes and a guarded x86_64
4 KiB page mapper for device MMIO. M013 is infrastructure only; it does not
initialize or enable any specific controller.

## Scope
- PCI config-space read/write APIs for 8-, 16- and 32-bit values using
  mechanism #1 (CF8/CFC).
- Bounds/alignment validation and interrupt-state preservation around config
  address/data transactions. Byte/word operations use native-width port I/O
  to avoid unintended writes to adjacent write-one-to-clear status bits.
- Explicit helper to set PCI Memory Space + Bus Master command bits for a
  discovered device with at least one memory BAR.
- Kernel-only MMIO mapping in a reserved high-half virtual window.
- MMIO leaf pages use PCD+PWT (uncacheable with the default x86 PAT layout).
- Rejects null/zero/oversized ranges, physical-address overflow, overlaps with
  Limine usable RAM, occupied virtual pages, and exhausted mapping slots.
- Exact-address/size unmap API. Intermediate page-table pages are retained.

## Safety
- No PCI device is modified during boot.
- Bus-master enable is an explicit API and must only be called after DMA
  buffers and controller ownership/state are ready.
- No live PCI BAR is mapped or dereferenced by M013 initialization.
- No xHCI reset/start, doorbell, interrupt, port, or USB enumeration activity.

## Limitations
- 4-level 48-bit canonical addressing; no LA57 support.
- 4 KiB pages only; existing large-page mappings are not split.
- Fixed 32-slot MMIO virtual window, 256 MiB per slot; at most 32 active
  mappings. Page-table pages allocated for MMIO are not reclaimed on unmap.
- Mapper relies on Limine HHDM to access page-table pages allocated by PMM.
- MMIO mapping API is not SMP-locked; current PSRE milestone assumes a single
  active CPU and serialized kernel use.
- UC selection relies on standard x86 PAT reset layout with PCD+PWT.
