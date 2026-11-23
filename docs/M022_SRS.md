# M022 — xHCI Configure Endpoint & Interrupt Transfer Engine

## Goal
Configure one descriptor-validated USB Interrupt IN endpoint for the already-enumerated M018 device and provide a bounded, polled transfer operation.

## Functional requirements
- Require M015 controller RUNNING and M018 device Address Device success.
- Accept an IN endpoint address, max packet size, and polling interval.
- Build an Input Context containing the active Slot/EP0 contexts plus the target endpoint context.
- Submit Configure Endpoint (xHCI command TRB type 12) and wait for Command Completion Event.
- Allocate a dedicated DMA transfer ring and transfer buffer; retain them for controller lifetime.
- Submit one Normal TRB, ring the endpoint doorbell, and wait for a matching Transfer Event.
- Return actual bytes using the residual length and enforce caller capacity.
- Limit each configured transfer ring to 255 transfers; no live ring wrap/reuse.

## CLI
- `m022-configure`: choose the first descriptor-validated HID Interrupt IN endpoint.
- `m022-read`: perform one transfer (up to 1024 bytes) and print received bytes in hex.
- `m022-info`: show endpoint and transfer counters.

## Constraints
- One endpoint configured per boot/controller lifecycle.
- Polling only; interrupts are not enabled.
- No HID report interpretation or keyboard/mouse input integration.
- No endpoint recovery/reset/stop command after timeout.
- DMA allocations remain pinned; do not free while controller may reference them.
