# M042 — Test Plan

- Structured M007 directory enumeration API strict compile.
- M009 queue metadata getter strict compile.
- M040 table row selection and consume-once behavior.
- M042 initialization against an M038 container.
- Six-row page population and 64-entry directory-cache contract.
- File selection checkbox gating for RAM staging and queue add.
- Files/Queue view switching and queue metadata display.
- Directory open and absolute path update.
- Regression host suites M032–M042.
- Strict freestanding C object compilation, C++ compilation, root Make dry-run,
  and cumulative ZIP integrity check.

Tests use host-side mocks for filesystem and GUI services; they do not prove
real ATA hardware, FAT media, framebuffer, QEMU, or physical-device behavior.
