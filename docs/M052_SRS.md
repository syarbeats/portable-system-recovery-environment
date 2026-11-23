# M052 — USB Hub Support & Multiple Device Management

## Goal
Provide a bounded USB 2.0 hub descriptor parser, hub topology registry, interrupt-IN change-bitmap decoder, and multi-device registry as a foundation for downstream hub enumeration.

## Requirements
- Parse class-specific hub descriptor type `0x29` with length, port-count, and bitmap-size validation.
- Support up to 8 registered hubs, 15 downstream ports per hub, 5 hub levels, and 32 device records.
- Validate parent hub/port relationships before registering a downstream hub or device.
- Decode hub interrupt bitmap bits 1..N into port-change events (bit 0 is hub status and is reserved for future handling).
- Track device identifiers, parent hub/port, xHCI slot, root port, VID/PID, and lifecycle state.
- Queue events in a bounded FIFO and count overflow.
- Keep serial recovery operational when no hub is registered.

## Non-goals in this milestone
No USB class control requests (`GET_DESCRIPTOR` for hub, `SET_FEATURE(PORT_POWER)`, `CLEAR_FEATURE(C_PORT_*)`), no hub interrupt endpoint transfer scheduling, no downstream xHCI `Enable Slot`/`Address Device`, no TT handling for full-/low-speed devices behind high-speed hubs, and no actual multi-device transport integration. Those require a per-slot/per-endpoint xHCI transfer architecture beyond the current single-device M017–M049 path.
