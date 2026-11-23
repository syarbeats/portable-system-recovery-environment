# M054 — Multi-Slot xHCI Device Context Registry

## Goal
Introduce a bounded per-slot registry and allocate independent DMA backing for each registered xHCI slot. Preserve existing M001–M053 behavior and keep recovery CLI available.

## Implemented
- Registry for up to 32 slots, capped by controller MaxSlotsEn.
- Per-slot metadata: slot ID, root port, speed, identity, state and generation.
- Independent 4 KiB device-context backing and 4 KiB EP0-ring backing per registered slot.
- DCBAA entry installation through M015.
- Slot state transitions and registry reporting.
- Offline initialization when xHCI is unavailable.

## Not yet implemented
- Enable Slot command orchestration in M054.
- Input Context construction and Address Device for each slot.
- Per-slot control-transfer submit/completion demultiplexing.
- Configure Endpoint, endpoint ring scheduling and transfer-event routing.
- Safe DMA reclamation and Disable Slot cleanup. Unregistered DMA is intentionally pinned.

M054 is a multi-slot memory/registry foundation, not yet a complete multi-device xHCI transport.
