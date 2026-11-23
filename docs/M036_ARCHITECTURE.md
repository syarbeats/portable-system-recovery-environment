# M036 Architecture

`m035/shell.c` is the desktop-facing launcher. It registers the four built-in app descriptors during shell initialization and delegates launches to `m036/app.c`.

`m036/app.c` owns a static registry of eight slots. Each slot stores lifecycle, M034 window ID, counters, a 64-byte opaque state area and a title snapshot. M034 remains responsible for window creation, focus, rendering, movement and destruction. M030 events continue to flow through M035; M035 then offers each event to M036, which dispatches only to the app whose window is focused.

The framework deliberately does not create processes or threads. A callback runs synchronously and must remain bounded. `psre_m036_reconcile()` detects windows closed by the window manager and changes their application state to STOPPED.

The four standard controls in this milestone are immediate-mode drawing helpers using M032/M033. They do not yet own widget state or receive pointer events.
