# Changelog — M038

- Added 8 statically allocated per-window widget containers.
- Bound each container to a running M036 app and its M034 window.
- Added 16 local widget slots per container with isolated state and IDs.
- Added client-area-relative coordinates and geometry validation.
- Added per-window mouse/keyboard event routing and focus.
- Added rendering that follows window movement and safely skips controls outside a resized client area.
- Added lifecycle reconciliation for stopped apps.
- Integrated container creation, rendering and input routing into M035.
- Added M038 host tests and design/build documentation.
