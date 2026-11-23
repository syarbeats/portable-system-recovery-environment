# M048 Test Plan

1. Build host syntax test: `make test-m048`.
2. Run all previous host suites M032-M047.
3. Cross compile with `x86_64-elf-gcc`; verify the new object is linked.
4. QEMU with framebuffer but no USB HID: verify boot continues and serial CLI remains available.
5. QEMU with USB xHCI emulation and opt-in enabled: verify M015/M016 status and confirm failures do not halt boot.
6. Physical PC: verify Limine framebuffer formats, controller handoff, USB keyboard/mouse enumeration, and GUI input. Do not mark pass until tested on target hardware.
