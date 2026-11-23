# CHANGELOG M050

- Added fixed-capacity PIT-driven cooperative event loop.
- Added periodic M049 GUI/input, M043 process and device-state tasks.
- Added nonblocking UART read and scheduler pumping while serial CLI waits.
- Added device status API and CLI status/pause/resume commands.
- Preserved serial recovery path and opt-in xHCI takeover.
- Explicitly deferred physical USB hot-plug/unplug detection and endpoint recovery.
