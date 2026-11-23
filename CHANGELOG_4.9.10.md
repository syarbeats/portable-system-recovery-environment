# Build 4.9.10 — flicker-free desktop
Cause: every mouse event called psre_m034_render(), which cleared the whole visible framebuffer pixel by pixel and
redrew the desktop, so the screen showed a blank/partial frame (flicker).
- Back buffer in M031, present once per loop (dirty rows only).
- Fast fill_rect.
- Test: tests/m060.
