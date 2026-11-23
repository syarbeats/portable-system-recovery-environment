# M038 — Window-Based Widget Container (SRS)

## Objective
Give each running M036 application an isolated widget container associated with its M034 window.

## Functional requirements
- Up to 8 containers, one per M034 window.
- Each container is bound to a registered, running M036 application and its current window ID.
- Up to 16 widgets per container.
- Widget IDs are local to a container.
- Local client-area coordinates; client origin is the inner window body origin.
- Label, Button, Textbox and Checkbox support.
- Mouse and keyboard events are routed only to the container belonging to the focused window.
- Per-container focus, text, checkbox state, event and redraw counters.
- Tab/Shift+Tab focus traversal; Enter activates Button/Checkbox; printable ASCII and Backspace edit Textbox.
- Rendering follows current M034 window position and dimensions.
- Widget creation rejects geometry outside the client area; rendering skips controls that no longer fit after resize.
- Container reconciliation removes associations when the M036 app stops or changes window.

## Constraints
Static memory, synchronous event handling, software rendering, no heap, no process isolation, no independent GPU surfaces, no alpha compositing, and no automatic layout engine.
