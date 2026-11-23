# M021 Build Validation

- M021 HID parser source/header added.
- M019 parser extended to retain HID class descriptor Report Descriptor length.
- CLI commands `m021-probe` and `m021-info` added.
- Root Makefile includes `kernel/src/m021/hid.c`.
- Strict host compile, C++ subsystem compile, Makefile dry-run and ZIP integrity are checked at packaging.
- No full x86_64-elf cross build, QEMU runtime, or physical USB validation is claimed.
