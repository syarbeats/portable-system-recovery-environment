# M036 — PSRE Application Framework: Software Requirements Specification

## Objective
Provide a small, deterministic application lifecycle and dispatch layer above M034 Window Manager and M030 Input Subsystem.

## Functional requirements
- Initialize a fixed-capacity registry for up to 8 applications.
- Register application descriptors containing title, initial window text and optional event callback.
- Launch a registered application in an M034 window; reuse/focus an existing live instance.
- Stop an application and destroy its associated window.
- Track lifecycle (`REGISTERED`, `RUNNING`, `STOPPED`, `ERROR`), window association, launch count, event count and 64-byte application state.
- Dispatch input events only to the application associated with the currently focused window.
- Reconcile application lifecycle when an M034 window is closed independently.
- Expose basic label, button, textbox and checkbox drawing controls.
- Integrate four PSRE built-in applications through M035.

## Constraints
- Static allocation only; no dynamic memory, process isolation, task scheduling or separate address spaces.
- Maximum 8 registered applications; one window per application instance.
- Application callbacks execute synchronously in the input dispatch path.
- State is volatile RAM and limited to 64 bytes per app.
- Controls are drawing primitives; focus, hit-testing, editing and accessibility semantics are not implemented.
