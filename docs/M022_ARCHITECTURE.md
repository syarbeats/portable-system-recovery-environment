# M022 Architecture

M022 uses M015's DMA allocator, command ring, event ring, MMIO doorbell and PIT timeout. It obtains the existing Device Context from M018 and constructs a new Input Context with the current Slot Context and EP0 Context copied into it. The target endpoint context is appended at its DCI index. The endpoint context uses the descriptor packet size for both Average TRB Length and Max ESIT Payload; SuperSpeed companion burst/multiplier parsing is not implemented, so this is a single-packet-per-service-interval foundation. A Configure Endpoint command activates that endpoint.

The endpoint transfer ring is allocated separately. Its final TRB is a Link TRB with Toggle Cycle. Each `m022-read` submits one Normal TRB into the ring, rings Doorbell[slot] with the DCI, polls M015's Event Ring for a matching Transfer Event, then copies the received DMA bytes to the caller.

The HID convenience function uses M021's parsed interface and M019's endpoint metadata to select an Interrupt IN endpoint. It does not configure alternate settings other than the default alternate setting already discovered by M021.
