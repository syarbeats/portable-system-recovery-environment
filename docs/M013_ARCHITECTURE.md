# M013 Architecture

## PCI configuration
`psre_pci_config_read/write{8,16,32}` access mechanism #1. Subword reads/writes use native `inb/inw` and `outb/outw` accesses at the
corresponding CFC byte lane, avoiding read-modify-write side effects on adjacent
write-one-to-clear status fields. `psre_pci_enable_mmio_busmaster`
checks that the BDF was discovered and has a nonzero memory BAR, then sets
Command.Memory Space Enable and Command.Bus Master Enable. It is intentionally
not called by boot code.

## MMIO virtual mapping
`vmm_map_mmio(physical_address, size)`:
1. validates the range and page-aligns its physical start;
2. rejects overlap with Limine usable RAM;
3. reserves a free range in the fixed high-half MMIO window;
4. walks/creates PML4 -> PDPT -> PD -> PT entries;
5. installs 4 KiB PTEs with Present, Writable, PCD and PWT. Existing parent
   page-table permissions are never broadened; an existing read-only or large
   mapping causes the candidate window to be rejected;
6. invalidates each virtual page with INVLPG;
7. returns a pointer preserving the caller's original physical offset.

`vmm_unmap_mmio(pointer, size)` validates an exact mapping record, clears leaf
PTEs and invalidates the corresponding TLB entries. Intermediate page-table
pages are retained to avoid reclaiming a table still referenced elsewhere.

## Boot integration
M013 runs a guard self-test using invalid arguments only. It does not access
hardware or allocate page tables during boot.
