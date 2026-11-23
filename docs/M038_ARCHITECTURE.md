# M038 — Architecture

`container.c` owns a static array of 8 containers. Each container stores M036 app ID, M034 window ID, focus and up to 16 widget records. Widget IDs are scoped to the container.

Creation validates that the app is registered and RUNNING, then binds the container to the app's current M034 window. Geometry is queried from M034 whenever controls are created or rendered. Client origin is `(window.x + 2, window.y + 24)`; available client dimensions exclude frame/title borders.

Event path: M034 processes input first and updates window focus/cursor. M035 then calls M038. M038 locates the container associated with the focused window, translates pointer coordinates into client-local coordinates and dispatches to that container only. If M038 does not consume the event, M035 can pass it to the legacy global M037 layer. M036 application event dispatch remains afterward.

Rendering draws non-focused containers first and the focused container last. Widgets are required to fit wholly inside the client rectangle. On window movement, the current origin is automatically used. On shrink, out-of-bounds controls are skipped and a render error is counted. `psre_m038_reconcile` removes containers whose app stopped or changed its window.
