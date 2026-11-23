# PSRE Build 2.6 — M031

- Added Limine framebuffer request and response structures.
- Added framebuffer validation for RGB 24/32-bpp, dimensions, pitch, channel masks, overlapping masks, overflow and bounded total size.
- Added pixel write and rectangle fill APIs using pitch and reported channel masks.
- Added explicit RGB test pattern command; framebuffer remains untouched during boot initialization.
- Added `m031-info` and `m031-pattern` CLI commands.
- Added M031 SRS, architecture, test plan, build validation and integration documentation.
- Preserved all M001–M030 source and project history.
