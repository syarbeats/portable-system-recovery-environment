# M020 Build Validation

- Added M020 API, implementation, and CLI commands.
- Added M018 EP0 no-data control transfer helper for standard OUT requests.
- Root Makefile includes `kernel/src/m020/usb_device.c`.
- Strict host compilation, C++ subsystem compile, root Makefile dry-run and ZIP integrity are checked at packaging time.
- No full cross build, QEMU runtime, or physical xHCI validation is claimed.
