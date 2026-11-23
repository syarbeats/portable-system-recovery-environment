# M034 — Window Manager Foundation (SRS)
## Goal
Provide a small, static-allocation 2D window manager over M031-M033 and route M030 keyboard/mouse events to window interactions.
## Functional requirements
- Support up to 8 windows with stable slot IDs.
- Create/destroy windows, set title, position, size and text.
- Maintain z-order and focused window.
- Render desktop, window frame, title bar, body, close affordance and software pointer.
- Raise window on click; drag by title bar; close by clicking title-bar close affordance.
- Send printable keyboard characters and Backspace to the focused window's text buffer.
- Clip drawing through M032/M033 backends and clamp windows to framebuffer bounds.
- Subscribe to M030 event dispatcher; do not poll hardware from the window manager callback.
## Limits
No compositor, alpha blending, GPU acceleration, resizable mouse edges, minimize/maximize, multiple desktops, taskbar, app process isolation, Unicode, or per-window off-screen surfaces. Rendering redraws the full framebuffer.
