# M026 Troubleshooting

- No endpoints active: verify M015 is RUNNING, M018 has assigned an address, M019 configuration parsing and M021 HID probing succeeded.
- Keyboard active, mouse inactive: check interface subclass/protocol (boot mouse is subclass 1, protocol 2), Interrupt IN endpoint descriptor and SET_PROTOCOL request.
- Mouse active, keyboard inactive: check boot keyboard interface (subclass 1, protocol 1) and its endpoint descriptor.
- Transfer timeout: check port/device stability, endpoint DCI, MPS, interval, xHCI event ring and controller state.
- Endpoint table full: M026 supports at most four Interrupt IN endpoints per addressed device.
- Poll stops after 255 submissions: transfer-ring wrap/reuse is intentionally not implemented yet.
- Input queue drops: consume events with `m026-next` (legacy `m025-next`) more frequently; FIFO capacity is 64.

M026 has not been validated against physical hardware in this build; treat runtime USB behavior as unverified until tested.
