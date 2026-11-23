# M013 Troubleshooting

- `vmm_map_mmio` returns NULL: check VMM initialization, HHDM, range size,
  physical range not overlapping usable RAM, canonical window availability,
  PMM capacity and mapping-slot count.
- `vmm_unmap_mmio` returns false: pass the exact pointer and exact original
  size returned to the caller.
- PCI config read returns false: verify mechanism #1 availability, BDF and
  DWORD-aligned offset for 32-bit operations.
- Do not enable bus mastering until DMA buffers are fully initialized and
  controller ownership/state is safe.
- Do not assume a BAR is safe to read or write. Consult the device programming
  specification and map it with `vmm_map_mmio` first.
