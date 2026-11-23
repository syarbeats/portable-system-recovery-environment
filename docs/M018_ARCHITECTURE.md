# M018 Architecture

M018 reuses M015's command/event rings and controller MMIO mapping. DMA is allocated through M015's below-4-GiB contiguous page allocator and intentionally remains pinned while xHCI is running.

Four pages are laid out as follows:

| Page | Use |
|---|---|
| 0 | Input Context (Input Control, Slot, EP0 contexts) |
| 1 | Device Context (referenced by DCBAA[Slot ID]) |
| 2 | EP0 Transfer Ring (Setup/Data/Status TRBs) |
| 3 | Device Descriptor data buffer |

The context stride is selected from HCCPARAMS1.CSZ (32 or 64 bytes). The Input Context adds Slot Context and EP0 Context, and Address Device is submitted on command doorbell 0. On successful command completion, a standard 18-byte GET_DESCRIPTOR(Device) control transfer is submitted through slot doorbell / endpoint target 1. M018 waits for a matching Transfer Event and parses the returned descriptor.

All operations are explicit CLI commands. No xHCI initialization or enumeration is performed automatically at boot.
