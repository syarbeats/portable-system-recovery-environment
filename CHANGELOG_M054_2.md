# CHANGELOG — Build 4.9.2 (M001–M054)

- Added detailed M031 Limine framebuffer response and validation diagnostics.
- Added explicit framebuffer failure reasons.
- Added guarded raw descriptor reporting for safe-to-inspect response entries.
- Added support for validated 16-bpp RGB framebuffer formats alongside 24/32-bpp.
- Added early RGB framebuffer write test before graphical boot log redraw.
- Added Limine `resolution: 1024x768` request to the PSRE entry.
- Added M031 host tests for missing response, RGB 16/24/32-bpp, unsupported BPP, and invalid pitch.
- Kept serial recovery and USB-independent GUI behavior.

Validation: M031 host test, all existing milestone host suites, strict syntax checks, Makefile dry-run, and archive integrity passed. QEMU OVMF and physical PC boots remain unverified.
