# M051 — USB Hot-Plug Detection & Device Lifecycle (Initial)

## Goal
Add bounded polling of xHCI PORTSC connection state and a kernel-visible lifecycle event queue while preserving the serial recovery path.

## Functional scope
- Read PORTSC through M016 without modifying port state or acknowledging RW1C change bits.
- Detect CCS connection-state transitions and CSC change indications.
- Track per-port states: EMPTY, CONNECTED, RESETTING, ENUMERATED, CONFIGURED, ACTIVE, DISCONNECTED, ERROR.
- Queue CONNECT/DISCONNECT events in a fixed 32-entry ring; count dropped events.
- Run scans from M050 every 50 PIT ticks.
- Expose `m051-info` and `m051-scan` CLI commands.

## Explicit limitations
This milestone detects port-level connection changes only. It does not yet disable an xHCI slot, free endpoint rings/DCBAA contexts, re-run Address Device, reconfigure HID endpoints, or safely re-enumerate after disconnect. State values beyond CONNECTED/DISCONNECTED are reserved for later lifecycle integration. CSC is observed but not acknowledged here; interrupt-driven xHCI Port Status Change Event processing is not implemented. One-device M049 state is not reset by M051.
