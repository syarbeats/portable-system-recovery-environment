# CHANGELOG M054

- Added bounded multi-slot xHCI registry (up to 32 slots, capped by MaxSlotsEn).
- Added per-slot device-context and EP0-ring DMA allocations and DCBAA entry registration.
- Added per-slot identity/state APIs and `m054-info` CLI command.
- Changed default M049 autostart to enabled and reordered boot so USB/HID GUI initialization is attempted before M048 readiness check.
- Preserved serial recovery fallback and added `M049_AUTOSTART=0` opt-out.
- Full multi-slot Address Device/control-transfer/endpoint routing and safe DMA release remain future work.
