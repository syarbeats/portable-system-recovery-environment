# M039 Architecture

M039 is a layout policy layer over M038. It does not own widget memory: it reads current widget geometry, computes positions, then applies geometry through `psre_m038_set_geometry()`.

- `layout.c`: vertical, horizontal and grid algorithms; per-container saved layout configuration; explicit reflow; one-container redraw; theme forwarding.
- M038 additions: geometry getter/setter, stable widget z-order list, bring-to-front, one-container rendering, theme palette.
- M035 integration: initialize M039 with the GUI shell and reapply saved layouts during shell redraw so resized windows are reflowed.

Rendering and hit testing follow the same z-order. The focused window remains ordered by M034; M039 only changes widget order inside its own container. Widgets whose full rectangle no longer fits the client area are skipped by M038. This is conservative whole-widget clipping; partial pixel clipping is not implemented.
