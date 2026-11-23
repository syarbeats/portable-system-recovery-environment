# M035 — PSRE GUI Shell (SRS)

## Objective
Deliver a basic desktop shell on top of M031 framebuffer, M032 graphics primitives, M033 font rendering, M034 window manager, and M030 input routing.

## Functional requirements
- Draw a dark-blue desktop background and static desktop icons.
- Draw a bottom taskbar with a Start button, quick-launch buttons and an uptime clock.
- Open/close a Start Menu and highlight its selected item.
- Launch Recovery Manager, System Information, USB Device Manager and Settings information windows.
- Support mouse clicks on Start, menu entries, desktop icons and quick-launch buttons.
- Support keyboard navigation in Start Menu using Up/Down, Enter to launch, Escape to close.
- Provide shell status and explicit redraw/poll CLI commands.
- Register a desktop background painter with M034 so wallpaper/icons are drawn before application windows.

## Constraints
- Four application windows are informational shells, not full recovery/system/USB management applications.
- Clock displays elapsed uptime based on the 100 Hz PIT tick counter, not wall-clock time or RTC time.
- Clock refresh occurs when the shell is redrawn or `m035-poll` is run; there is no background scheduler yet.
- Desktop icons and taskbar are static; no drag/drop, context menu, dynamic icon layout or persistent preferences.
- No alpha blending, GPU acceleration, compositor, application process isolation or independent app lifecycle.
- GUI is explicitly started from the serial CLI after M030 input initialization.
