# PSRE M002 Architecture

## Boot contract
Limine provides the memory map and HHDM response. The kernel uses base revision 3. Protocol response pointers are already virtual pointers; do not add HHDM to them. Physical page-table addresses are converted to virtual addresses using `HHDM + physical`.

## Physical memory manager
- 4 KiB page granularity.
- Two bitmaps: `used` and `allocated`.
- Initial state: every page unavailable.
- Only full pages within `LIMINE_MEMMAP_USABLE` entries become free.
- First 1 MiB remains reserved conservatively.
- `allocated` distinguishes PMM-owned frames from firmware/kernel-reserved frames and prevents accidental freeing of reserved pages.
- Search uses a next-fit hint and wraps once.

## Kernel heap
A 256 KiB statically reserved arena, 16-byte aligned, with first-fit allocation, splitting, and adjacent free-block coalescing. It is deliberately independent from PMM for this milestone, avoiding a bootstrap dependency.

## Virtual memory diagnostics
Reads CR3 and walks the active x86-64 four-level page tables through the HHDM. Supports 4 KiB, 2 MiB, and 1 GiB leaf mappings. It does not modify page tables.

## Memory ownership
- Limine `USABLE`: eligible for PMM after page alignment.
- All other map types: unavailable.
- Limine response objects and bootloader-reclaimable data are not reclaimed.
- Kernel image and static PMM/heap buffers remain reserved by the bootloader memory map.
