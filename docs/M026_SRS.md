# M026 — USB HID Multi-Endpoint Support: Software Requirements

## Goal
Extend the M022 interrupt-IN transfer engine and M025 input manager so one enumerated USB device can expose and use a boot keyboard and a boot mouse concurrently.

## Functional requirements
1. Support up to four independent HID Interrupt IN endpoints in one xHCI slot.
2. Give each endpoint its own DMA allocation, transfer ring, enqueue index, transfer buffer, endpoint DCI, packet size and counters.
3. Configure each endpoint using Configure Endpoint without replacing the other configured endpoints.
4. Submit and poll a transfer by endpoint address.
5. M025 initializes keyboard and mouse independently; either may remain operational if the other is absent or fails configuration.
6. M025 polls each active endpoint once per `m026-poll` call and merges generated events into the existing 64-entry FIFO.
7. Preserve legacy M022/M025 entry points and CLI commands.

## Constraints
- Single enumerated USB device/slot; multi-device registry is not part of M026.
- HID boot keyboard and boot mouse only.
- Maximum four Interrupt IN endpoints; no live transfer-ring wrap/reuse (255 usable entries per endpoint).
- Polling is synchronous and explicit; no background scheduler or interrupt-driven input.
- DMA allocations remain reserved for controller lifetime; no endpoint teardown/reclamation.
- USB source device remains subject to existing M017–M022 lifecycle limitations.
