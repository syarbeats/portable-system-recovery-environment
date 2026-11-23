# PSRE Build 1.9 — M022 Release Notes

## Added
- xHCI Configure Endpoint command path for one descriptor-validated Interrupt IN endpoint.
- Input Context construction preserving the active Slot and EP0 contexts.
- Dedicated DMA transfer ring and transfer buffer.
- Polled Interrupt IN Normal TRB submission and Transfer Event completion.
- HID endpoint selection using M019 configuration metadata and M021 HID discovery.
- CLI: `m022-configure`, `m022-read`, `m022-info`.

## Limitations
- One Interrupt IN endpoint per controller/device lifecycle.
- Transfer ring intentionally stops after 255 transfers; no live ring wrap.
- No HID report decoding, keyboard/mouse event manager, interrupt-driven event handling, endpoint recovery, USB Mass Storage, or multi-device lifecycle.
- No QEMU or physical hardware runtime validation in this build.
