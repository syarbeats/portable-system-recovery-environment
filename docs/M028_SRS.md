# M028 Software Requirements Specification

## Goal
Provide an advanced USB HID mouse driver using HID Report Protocol, integrated with the existing M026 multi-endpoint xHCI polling engine and M025 event queue.

## Functional requirements
1. Locate a dedicated HID mouse interface identified by M021.
2. Retrieve and parse its HID Report Descriptor with bounded memory and item validation.
3. Select Report Protocol for boot-capable HID mouse interfaces.
4. Decode report IDs and bit fields for relative X, relative Y, vertical wheel, horizontal pan, and buttons 1–8; ignore unsupported absolute axes.
5. Reject malformed or undersized reports without reading beyond the report buffer.
6. Publish movement, button, vertical-wheel and horizontal-wheel events through M025.
7. Expose driver status through `m028-info`.

## Non-functional requirements
- No dynamic allocation.
- Fixed upper bounds: 512-byte descriptor, 32 mouse fields, 16 report IDs.
- Signed values are sign-extended and clamped to the public event data range.
- Existing M001–M027 sources remain in the cumulative archive.

## Out of scope
Absolute coordinates, >8 buttons, high-resolution wheel scaling, multiple mouse interfaces, composite keyboard+mouse interface support, transfer-ring wrap/recovery, and runtime hardware certification.
