# M049 Test Plan

1. Run `make test-m049` for safe-default source/boot/CLI contract checks.
2. Run strict freestanding syntax checks for M049, `main.c`, and M006 CLI.
3. Run all prior host test suites M032–M048 where present.
4. Dry-run `make -n all` and `make -n M049_AUTOSTART=1 all`.
5. Hardware test only after reviewing xHCI ownership/takeover implications: boot with autostart off; then test with a directly attached USB HID keyboard; next test a directly attached mouse; finally test a combined keyboard/mouse device.
6. Verify missing device, non-HID device, unsupported report descriptor, and failed endpoint setup all leave serial CLI responsive.

Physical xHCI, HID transfer, and framebuffer tests are not replaced by host syntax tests.
