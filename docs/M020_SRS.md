# M020 — USB Configuration Activation & Device Class Identification

## Goal
Activate the first configuration parsed by M019, support explicit interface alternate-setting selection, classify interfaces by USB class code, and register the currently enumerated device.

## Functional scope
- Require successful M018 Device Descriptor and M019 Configuration Descriptor processing.
- Send standard device request `SET_CONFIGURATION` (bmRequestType 0x00, bRequest 9).
- Register slot, root port, speed, VID, PID, selected configuration, and interface count.
- Classify interface descriptors:
  - 0x03 HID
  - 0x08 Mass Storage
  - 0x09 Hub
  - 0x02 / 0x0A CDC
  - 0xFF vendor-specific
  - 0x00 unknown / interface-defined
  - remaining values other
- Support explicit `SET_INTERFACE` (bmRequestType 0x01, bRequest 11) only for interface/alternate pairs present in M019 descriptors.
- Maintain a bounded registry with four entries; current implementation populates only entry zero because M017/M018 support one active device lifecycle.

## CLI
- `m020-configure`
- `m020-alt INTERFACE ALTSETTING`
- `m020-info`

## Limitations
- No HID, Mass Storage, Hub or CDC class driver.
- No endpoint transfer engine beyond EP0 control transfers.
- No multi-device hotplug lifecycle.
- Alternate-setting CLI parser currently accepts positive decimal values only; use descriptor-listed values.
