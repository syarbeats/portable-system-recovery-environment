# M019 — USB Configuration & Interface Discovery

## Goal
After M018 has successfully addressed one USB device and read its Device Descriptor, M019 retrieves and parses the first Configuration Descriptor, Interface Descriptors, Endpoint Descriptors, and available string descriptors.

## Functional requirements
- Require successful M018 Device Descriptor stage.
- Request the 9-byte Configuration Descriptor header.
- Validate `wTotalLength` (9..1024 bytes).
- Retrieve and parse the complete configuration descriptor.
- Parse Interface (type 4) and Endpoint (type 5) descriptors with strict length/bounds checks.
- Retrieve language ID and Manufacturer/Product/Serial strings when descriptor indices are nonzero.
- Limit to 16 interface descriptors and 32 endpoint descriptors.
- Convert UTF-16LE string data to printable ASCII, replacing unsupported characters with `?`.

## CLI
- `m019-config`: perform descriptor reads and parsing.
- `m019-info`: report parsed configuration, interfaces, endpoints and strings.

## Limitations
- First configuration only; no SET_CONFIGURATION request is sent.
- No alternate-setting selection, hub support, class drivers, HID, BOT/SCSI, or filesystem mounting.
- String conversion is ASCII-only.
- Configuration descriptor length is capped at 1024 bytes.
