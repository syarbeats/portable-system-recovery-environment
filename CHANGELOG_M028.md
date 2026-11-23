# PSRE Build 2.3 — M028 Advanced USB HID Mouse

## Added
- HID Report Protocol mouse driver with bounded Report Descriptor retrieval and parsing.
- Report ID-aware bitfield decoding for relative X/Y, vertical wheel, Consumer Page horizontal pan, and buttons 1–8.
- Signed field extension, relative-axis filtering and endpoint maximum-packet-size validation.
- Signed field decoding and clamping for movement and wheel values.
- M025 input manager integration and new `MOUSE_WHEEL` / `MOUSE_HWHEEL` events.
- `m028-info` diagnostics.

## Constraints
- M028 selects a dedicated HID mouse interface (`kind=MOUSE`). Composite keyboard+mouse interfaces are not yet supported by the M028 driver because switching the interface to Report Protocol would conflict with the current Boot Keyboard driver.
- Buttons above 8 are ignored.
- Absolute pointer coordinates, high-resolution wheel multipliers, vendor-specific usages, multiple mouse interfaces, and HID report descriptor global/local constructs beyond the bounded parser are not implemented.
- M022 transfer rings still have a finite per-endpoint submission count; ring wrap/recovery is not part of M028.
- No claim of QEMU or physical hardware runtime validation.

## Synthetic parser checks
- Standard 3-button wheel mouse descriptor without Report IDs: PASS (buttons, negative X, positive Y, negative wheel).
- Report-ID descriptor with eight buttons and Consumer Page horizontal pan: PASS.
