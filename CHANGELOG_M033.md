# PSRE Build 2.8 — M033

- Added built-in 8x8 and 8x16 fixed-cell ASCII bitmap fonts (0x20–0x7E).
- Added glyph drawing with foreground color and optional opaque background.
- Added string rendering with newline, carriage return, tab, character spacing, and bounded text regions.
- Added text measurement and framebuffer-edge clipping.
- Added `m033-init`, `m033-info`, and `m033-demo` CLI commands.
- Added M033 SRS, architecture, test plan, validation notes, integration notes, and font attribution/license.
- Added `make test-m033` host test target.
- Preserved M001–M032 source tree and root Makefile.
