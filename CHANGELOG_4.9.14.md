# Build 4.9.14 — larger desktop text
- New M033 UI large mode (8x16 glyphs cropped to 13 rows for 8x8 requests); same 9 px advance, 14 px line pitch.
- Enabled by psre_m035_initialize(); runtime switch: `m033-font normal|large`.
- Clipping fixes in M035: icon labels (box was 68 px for up to 11 characters), clock (80 px box), controls anchored
  to the bottom of the window so they never collide with the taller body text.
- Tests: tests/m033 (large mode), tests/m035.
