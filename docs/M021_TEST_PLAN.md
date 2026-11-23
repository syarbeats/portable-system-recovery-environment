# M021 Test Plan

## Preconditions
1. Boot PSRE with serial console.
2. Run M014 probe and M015 initialization.
3. Reset a connected port with M016.
4. Run M017 Enable Slot and M018 enumeration.
5. Run M019 configuration parsing.
6. Run M020 configuration activation.

## HID descriptor test
1. Connect a USB HID keyboard or mouse.
2. Run `m021-probe`.
3. Run `m021-info`.
4. Confirm interface number, Interrupt IN endpoint address, packet size, interval, report descriptor length and report ID input sizes.

## Negative tests
- Run before M020 configuration activation: reject.
- HID interface without HID descriptor: reject.
- No Interrupt IN endpoint: reject.
- Report Descriptor length 0 or >512: reject.
- Truncated/invalid HID item stream: reject.
- Excessive Report IDs or report bit sizes: reject.

## Runtime caveat
No interrupt endpoint transfers or input events are implemented in M021. QEMU and physical runtime tests are required for descriptor request validation.
