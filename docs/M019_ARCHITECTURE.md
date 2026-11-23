# M019 Architecture

M019 reuses the Endpoint 0 control-IN transfer path established by M018. M018 exposes a bounded helper that submits Setup/Data/Status Stage TRBs on the existing EP0 transfer ring, rings the slot's EP0 doorbell, and waits for a matching Transfer Event.

M019 requests the first configuration descriptor header, validates `wTotalLength`, requests the full descriptor, then walks the descriptor stream using each descriptor's `bLength`. Interface and endpoint data are copied into bounded static status arrays. String descriptors are requested using the first advertised language ID where available.

The EP0 transfer ring is shared with M018. M018 starts subsequent transfers after the three TRBs used for the Device Descriptor request; M019 advances the producer index by three TRBs per control transfer. The ring's final TRB is a Link TRB with Toggle Cycle.

All control transfers are synchronous and polled. DMA allocations remain pinned while the xHCI controller may reference them.
