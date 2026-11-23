# M035 — Test Plan

1. Reject initialization unless M030, M034 and M031 are ready.
2. Verify the desktop painter is registered and invoked by M034.
3. Verify taskbar and Start Menu rendering on a synthetic framebuffer.
4. Toggle Start Menu and test selection wrapping with Up/Down.
5. Test Enter launches the selected app and Escape closes the menu.
6. Launch all four app information windows and verify their content.
7. Test mouse hit regions for Start, menu rows, desktop icons and quick-launch buttons.
8. Verify uptime conversion from a synthetic 100 Hz PIT tick counter.
9. Run M032/M033/M034/M035 host tests and strict freestanding compilation.
10. Boot via Limine in QEMU and test USB keyboard/mouse on physical hardware.
