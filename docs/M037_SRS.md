# M037 — Interactive GUI Widget System (SRS)
## Purpose
Provide reusable interactive controls for PSRE GUI applications using M030 input and M032/M033 drawing.
## Functional requirements
- Static registry: maximum 32 widgets.
- Label, Button, Textbox and Checkbox controls.
- Enabled/disabled state; only enabled interactive controls receive focus.
- Mouse hit testing and hover feedback from M030 absolute pointer state.
- Tab/Shift+Tab focus traversal; Enter activates buttons and toggles checkboxes.
- Textbox supports printable ASCII, Backspace and bounded input length.
- Buttons provide pressed visual feedback and optional action callback.
- Checkbox state can be queried and changed programmatically.
## Constraints
Widgets use framebuffer-relative absolute coordinates. Registry is global, static and synchronous. No per-window clipping/ownership, heap allocation, Unicode shaping, clipboard, selection, text scrolling or cursor navigation.
