# M014 Architecture

M014 is a read-only bridge between M004 PCI discovery, M013 MMIO mapping, and
M012's pure capability parser.

1. Scan the cached M004 PCI inventory for the first xHCI function.
2. Select the first discovered memory BAR and verify page alignment.
3. Map `0x20` bytes at the BAR base using M013.
4. Read seven DWORDs (offsets `0x00` through `0x18`) into a local byte array.
5. Pass the `0x1C`-byte snapshot to M012.
6. Unmap the temporary mapping and record the remaining mapping count.
7. Publish parsed capability fields only if parsing and unmapping succeed.

No live BAR pointer is retained. The module does not issue DMA or interact with
Operational, Runtime, Doorbell, or Port registers. It is intentionally not an
xHCI controller driver yet.
