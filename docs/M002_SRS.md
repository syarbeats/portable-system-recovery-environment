# PSRE M002 — Software Requirements Specification
Version 0.2 | Status: implementation baseline

## Objective
Implement the first usable memory-management foundation for the x86_64 freestanding kernel.

## Functional requirements
- M002-FR-01: obtain the Limine memory-map response.
- M002-FR-02: obtain the Higher Half Direct Map (HHDM) offset.
- M002-FR-03: classify usable RAM and retain all non-usable regions as unavailable.
- M002-FR-04: allocate and free aligned 4 KiB physical page frames.
- M002-FR-05: reject unaligned, out-of-range, and double-free page releases.
- M002-FR-06: provide a bounded kernel heap with allocation and free/coalescing.
- M002-FR-07: read CR3 and translate currently mapped virtual addresses for diagnostics.
- M002-FR-08: print memory statistics over COM1.
- M002-FR-09: execute PMM and heap allocation/free self-tests during boot.

## Non-functional requirements
- Freestanding x86_64; no hosted C/C++ runtime.
- No dynamic allocation before PMM/heap initialization.
- Page size: 4096 bytes.
- Physical address management cap: 64 GiB in this milestone.
- The allocator must never return a page outside a Limine USABLE memory-map region.
- No bootloader-reclaimable memory is reclaimed in M002.
- Fail safely when required Limine responses are absent.

## Scope and limitations
- PMM manages up to 64 GiB of physical address space. RAM above the cap is ignored, not treated as free.
- Heap is a fixed 256 KiB statically reserved kernel arena; it does not yet grow by requesting more pages.
- VMM is diagnostic-only in M002: CR3 discovery and page-table walking. Creating/removing mappings and changing page-table permissions are deferred.
- Bootloader reclaimable regions remain reserved for the lifetime of this milestone.
- SMP locking, NUMA, memory hotplug, swap, user-space address spaces, and demand paging are out of scope.

## Acceptance criteria
1. Kernel receives valid Limine memory-map and HHDM responses.
2. PMM initializes and reports non-zero free pages in QEMU with RAM configured.
3. One physical page can be allocated and freed successfully.
4. Double-free and unaligned free are rejected.
5. A 128-byte heap allocation succeeds and can be freed.
6. Heap coalescing restores the free-space count after the self-test.
7. CR3 is read and printed; page-table translation is available for mapped addresses.
8. Kernel prints `M002 PASS` and halts without triple fault.

## Risks
- Limine request ABI/section layout must match the selected Limine release.
- Incorrect HHDM assumptions can fault during page-table inspection.
- A physical memory cap that is too low will omit high RAM; it must be raised deliberately with bitmap sizing changes.
- The static heap is intentionally small and is not a general-purpose production allocator.
