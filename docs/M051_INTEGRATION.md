# M051 Integration

- `kernel/include/psre/m051/device_lifecycle.h`: lifecycle states, event and status API.
- `kernel/src/m051/device_lifecycle.c`: bounded port scan, state transitions and event FIFO.
- `kernel/include/psre/m016/usb_ports.h` / `kernel/src/m016/usb_ports.c`: read-only PORTSC accessor.
- `kernel/src/m050/event_loop.c`: periodic M051 scan every 50 PIT ticks.
- `kernel/src/main.c`: M051 initialization after M049 and before M050 scheduler start.
- `kernel/src/m006/cli.c`: `m051-info`, `m051-scan`.

Events can be consumed using `psre_m051_next_event()`.
