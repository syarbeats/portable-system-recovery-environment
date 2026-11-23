# M031 — Test Plan

1. Boot with 24-bit RGB framebuffer; verify geometry/masks.
2. Repeat with 32-bit RGB and non-default pitch.
3. Run `m031-pattern`; verify red/green/blue bars.
4. Reject absent/null response, zero/oversized dimensions, unsupported bpp/model, invalid masks, short pitch, overflow, and framebuffer size above 1 GiB.
5. Verify initialization alone does not change framebuffer bytes.
6. Verify serial CLI remains available without a framebuffer.

Host compilation and synthetic structure tests do not replace QEMU or physical display testing.
