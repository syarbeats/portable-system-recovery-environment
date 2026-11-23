# M032 — Architecture

M031 owns framebuffer discovery, pixel layout, pitch and bounded physical pixel writes. M032 is a software drawing layer that calls M031 for each pixel/rectangle operation. Geometry helpers clip rectangles and lines to the visible framebuffer. Triangle fill uses signed 64-bit edge functions; circle operations use the midpoint algorithm; lines use Bresenham after Cohen-Sutherland clipping. Bitmap blitting accepts raw RGB24 or BGR24 pixels and does not parse file formats.

M032 is initialized explicitly by `m032-demo` in this milestone; kernel boot remains display-neutral. No framebuffer contents are changed by initialization.
