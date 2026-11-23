# M033 — Font Rendering Engine (SRS)

## Goal
Render readable bitmap text to the framebuffer using the M032 graphics API.

## Requirements
- Built-in 8x8 and 8x16 bitmap fonts.
- Printable ASCII glyph range 0x20–0x7E; unsupported bytes render as `?`.
- Foreground color and optional opaque background.
- String rendering with X/Y origin, character spacing, newline, carriage return, and tab.
- Optional width/height bounds on text layout.
- Text measurement API.
- Per-pixel clipping against framebuffer boundaries.
- Serial diagnostics and explicit graphical demonstration.

## Non-goals
No Unicode, UTF-8 shaping, proportional fonts, anti-aliasing, font file loading, terminal scrollback, or GUI text input widgets.
