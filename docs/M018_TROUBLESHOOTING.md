# M018 Troubleshooting

- `Run M015 init and successful m017-enable-slot`: M018 requires the exact same port's successful M017 slot allocation.
- `Invalid xHCI context size`: HCCPARAMS1.CSZ was not readable while M015 is running.
- `DMA allocation failed`: insufficient contiguous physical pages below the DMA limit or unavailable HHDM.
- Address Device failure: verify M017 Slot ID, DCBAA entry, Input Context Add Context Flags, Slot Context root port/speed, EP0 context and command completion code.
- Device Descriptor failure: verify EP0 transfer ring dequeue pointer, cycle state, Setup/Data/Status TRBs, endpoint doorbell target and transfer completion event.
- Do not free M018 DMA pages while the controller may still reference them. Reinitialize/halt the controller through a future explicit teardown path before reclaiming them.
