# M021 Troubleshooting

- **M019/M020 prerequisite missing:** parse and activate the configuration first.
- **No HID interface:** the active configuration may not contain Interface Class 0x03.
- **HID descriptor missing:** configuration descriptor did not contain a valid HID class descriptor with a Report Descriptor entry.
- **No Interrupt IN endpoint:** inspect M019 endpoint output and interface endpoint association.
- **Report Descriptor rejected:** inspect item lengths, Report Size/Count and Report IDs; M021 applies conservative limits.
- **No key/mouse input:** expected in M021; interrupt transfer support and event decoding are not implemented yet.
