# M020 Troubleshooting

- **M018/M019 prerequisite:** successful Device Descriptor and Configuration Descriptor processing are required.
- **SET_CONFIGURATION timeout/failure:** check EP0 transfer/event handling and that the selected configuration value is present in M019 output.
- **SET_INTERFACE rejected:** the interface number and alternate setting must occur in the parsed descriptors.
- **No class driver:** M020 only identifies classes; it does not yet operate HID, Mass Storage, Hub or CDC functions.
- **Single-device limitation:** current M017/M018 lifecycle populates only one registry record.
