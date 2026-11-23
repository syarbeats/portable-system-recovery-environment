# PSRE Integrated Build M001–M050

This release retains all M001–M049 files and adds M050 cooperative scheduling and HID device-state reporting. The kernel uses the M003 PIT tick, M049 GUI polling and M043 process tick. The serial CLI remains active and cooperatively services the scheduler while waiting for UART input.

USB controller takeover remains opt-in through `M049_AUTOSTART=1`. The M050 device manager is not a true USB hot-plug driver; port-change event processing is deferred.
