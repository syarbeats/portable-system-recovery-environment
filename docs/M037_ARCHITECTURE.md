# M037 — Architecture
`m037/widget.c` owns a fixed 32-entry registry. Each entry stores type, bounds, enabled state, text, optional callback and type-specific state. Dependencies: M030 pointer state, M032 primitives, M033 text rendering.

Event path: USB HID -> M025 -> M030 -> M035 subscriber -> M037 dispatch. M035 sends each event to M037 after desktop/window handling and before M036 app dispatch. Focus is one widget ID. Tab traverses enabled non-label widgets in registry order; Shift+Tab reverses using the modifier bit. Mouse hit testing and hover state use M030 absolute coordinates.

M037 is a global framebuffer-coordinate widget layer, not a retained-mode layout engine. It does not clip widgets to M034 windows or bind ownership to M036 applications. Applications must position widgets in non-overlapping screen regions and call render after view changes.
