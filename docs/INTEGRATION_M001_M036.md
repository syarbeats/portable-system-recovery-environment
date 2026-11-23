# PSRE Integration M001–M036

This cumulative release retains the M001–M035 source tree and adds M036 Application Framework. The root Makefile remains the single build entry point. M035 initializes M036 and registers Recovery Manager, System Information, USB Device Manager and Settings. M035 launcher actions create/focus framework-managed M034 windows. Input events are offered to the framework after shell handling; the framework routes them to the application associated with the focused window.

CLI: `m036-info` reports framework counters. Start the GUI using the existing sequence: initialize USB/HID and M030 input, then `m035-start`; M035 initializes M034/M036 as appropriate.
