# M032 — Graphics Primitive Engine (SRS)

## Objective
Provide a small freestanding 2D software graphics API on top of M031's validated Limine framebuffer.

## Functional requirements
- Initialize only when M031 reports a valid framebuffer.
- Draw and clear RGB pixels/regions.
- Draw clipped lines, rectangle outlines and filled rectangles.
- Draw circle outlines and filled circles.
- Draw outlined and filled triangles.
- Blit clipped RGB24/BGR24 bitmap images with caller-supplied stride.
- Reject invalid dimensions, unsupported bitmap formats and extreme coordinates.
- Expose diagnostic status and explicit CLI initialization/diagnostic demonstration.

## Constraints / non-goals
No alpha blending, font rendering, GPU acceleration, double buffering, page flipping, display mode changes, image file decoding, anti-aliasing, or automatic drawing during boot. Drawing is synchronous and writes directly to the framebuffer.
