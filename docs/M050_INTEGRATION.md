# M050 Integration

- `kernel/include/psre/m050/event_loop.h`: task API and scheduler status.
- `kernel/src/m050/event_loop.c`: fixed-capacity cooperative scheduler.
- `kernel/include/psre/m050/device_manager.h`: device snapshot API.
- `kernel/src/m050/device_manager.c`: M049-derived HID state snapshot.
- `kernel/src/serial.c`: nonblocking UART character read.
- `kernel/src/m006/cli.c`: scheduler pump while waiting for serial input; `m050-info`, `m050-devices`, `m050-pause`, `m050-resume`.
- `kernel/src/main.c`: initializes device manager and scheduler after M049.

The scheduler can register M049 input polling lazily if the GUI becomes ready after boot (for example, after a CLI start command).
