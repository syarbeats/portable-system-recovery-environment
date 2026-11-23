# M037 Integration
- Source: `kernel/src/m037/widget.c`
- API: `kernel/include/psre/m037/widget.h`
- Host test: `make test-m037`
- M035 initializes M037 and routes input events to `psre_m037_handle_event`.
- Widget IDs are independent of M034 window IDs and M036 application IDs.
- Widget coordinates are global framebuffer coordinates; no window clipping/ownership.
