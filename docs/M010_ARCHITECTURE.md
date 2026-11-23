# M010 — Architecture
`kernel/src/m010/usb.c` consumes the existing read-only PCI device inventory (`psre_pci_get_devices`). It filters class/subclass 0x0C/0x03, classifies controllers by programming interface and prints BAR metadata. `kernel/src/m004/init.c` invokes this after PCI enumeration. The CLI command `usb-info` reprints the inventory.

No new PCI configuration writes or MMIO accesses are made. This is intentional: the current kernel does not yet have a dedicated MMIO mapping/cache policy, DMA-safe allocation, IOMMU handling, xHCI command/event ring implementation, USB protocol stack, or USB Mass Storage BOT/SCSI driver.
