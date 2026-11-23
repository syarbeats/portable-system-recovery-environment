# M033 — Architecture

M033 stores two fixed-width ASCII bitmap tables (8x8 and 8x16). The renderer resolves each input byte to a glyph, maps set bits to foreground pixels, and optionally paints unset bits with the background color. Pixel writes are delegated to M032, which delegates to M031's framebuffer pixel writer. Text coordinates are clipped per pixel; layout supports newline, carriage return, tab, spacing, and optional width/height limits.
