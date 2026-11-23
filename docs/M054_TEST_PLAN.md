# M054 Test Plan

- Host test: offline registry initialization without an xHCI controller.
- Compile test: freestanding syntax of M054, main and CLI.
- Integration review: Makefile source registration and `test-m054` target.
- Hardware tests still required: multi-slot DMA/DCBAA registration, independent Address Device, per-slot EP0 control transfers, endpoint setup, device removal and safe DMA cleanup.
- Boot tests still required: UEFI ISO boot in QEMU and physical systems with supported xHCI hardware, framebuffer and USB HID.
