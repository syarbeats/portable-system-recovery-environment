# M031 — Architecture

Limine framebuffer request -> response validation -> metadata -> bounded software pixel writer. The address supplied by Limine is treated as its boot-provided virtual address; M031 does not remap it through the PCI MMIO mapper. Pixel values are assembled from channel masks and written little-endian using bytes-per-pixel and pitch. Initialization runs after memory-map/HHDM acquisition and does not draw. `m031-pattern` is explicit.
