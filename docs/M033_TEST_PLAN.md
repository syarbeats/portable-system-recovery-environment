# M033 — Test Plan

1. Initialize against a synthetic framebuffer.
2. Draw representative 8x8 and 8x16 glyphs; verify framebuffer changes.
3. Draw opaque and transparent text.
4. Verify newline and text measurement for multiple lines.
5. Draw a glyph partially outside the left edge; ensure safe clipping.
6. Verify unsupported control/non-ASCII bytes map to `?`.
7. Run `make test-m033`; then validate with Limine + QEMU and a physical display.
