# M021 — USB HID Descriptor & Interface Foundation

## Goal
Discover HID interfaces in the active USB configuration, validate their Interrupt IN endpoint, retrieve their HID Report Descriptor over EP0, and parse basic report layout metadata.

## Functional requirements
- Require successful M019 configuration parsing and M020 SET_CONFIGURATION.
- Identify Interface Class 0x03, Alternate Setting 0.
- Parse HID class descriptor (type 0x21) from the configuration descriptor stream and extract Report Descriptor length (type 0x22).
- Validate and record one Interrupt IN endpoint per interface.
- Retrieve Report Descriptor using GET_DESCRIPTOR(Report), recipient Interface (bmRequestType 0x81), and interface number in wIndex.
- Parse HID short and long items with bounds checks.
- Track Report ID, Input report size/count, input bits/bytes.
- Recognize Generic Desktop Mouse usage (Usage Page 0x01, Usage 0x02) and Keyboard usage page (0x07).
- Bound report descriptor to 512 bytes, HID interfaces to 8, Report IDs to 16.

## CLI
- `m021-probe`
- `m021-info`

## Limitations
- This milestone is descriptor discovery/parser foundation only.
- It does not issue Configure Endpoint, does not create an Interrupt IN Transfer Ring, and does not poll endpoint data.
- No keyboard scan code or mouse event processing yet.
