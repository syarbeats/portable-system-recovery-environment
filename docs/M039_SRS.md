# M039 — GUI Layout & Rendering Engine (SRS)

## Objective
Provide deterministic automatic widget placement and reusable layout behavior for M038 window containers, with explicit per-window redraw and configurable widget theme.

## Functional requirements
- FR-01: Vertical layout positions widgets from top to bottom using padding and vertical gap.
- FR-02: Horizontal layout positions widgets from left to right using padding and horizontal gap.
- FR-03: Grid layout supports a caller-selected column count and spacing.
- FR-04: Stretch mode expands widget width to the available cell/client width.
- FR-05: A successful layout is retained per container and can be reapplied after a window resize through `psre_m039_reflow_all()`.
- FR-06: A container can be redrawn independently.
- FR-07: Widget z-order can be changed explicitly; hit-testing and rendering use that order.
- FR-08: Theme colors can be replaced at runtime.
- FR-09: Widgets outside the resized client area are suppressed by M038 bounds validation.

## Limits
Static allocation only; maximum 8 containers and 16 widgets/container. No constraints solver, nested layouts, percentage units, animation, alpha blending, partial pixel clipping, or background layout thread. Layout calls are synchronous. The first implementation uses each widget's existing height; grid rows are equally spaced while widget heights remain intrinsic.
