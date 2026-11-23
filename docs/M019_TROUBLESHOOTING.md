# M019 Troubleshooting

- **M018 prerequisite missing:** run the M015/M016/M017/M018 sequence first.
- **Configuration header request fails:** check device connection, port reset, M017 slot allocation, and M018 Device Descriptor success.
- **Invalid total length:** device returned an invalid or unsupported descriptor length; M019 caps the complete configuration at 1024 bytes.
- **Malformed descriptor layout:** inspect `bLength` and descriptor type fields. The parser rejects truncated descriptors.
- **String descriptor failure:** some devices omit optional strings or have nonstandard descriptors. M019 currently treats a failed requested string read as an incomplete M019 operation.
- **No QEMU/hardware confirmation:** successful compilation alone is not proof of working USB transfers.
