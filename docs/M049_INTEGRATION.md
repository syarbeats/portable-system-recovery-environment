# M049 Integration Guide

- Default safe build: `make clean all`.
- Explicit USB auto-start build: `make clean all M049_AUTOSTART=1`.
- Boot option is compiled into the kernel; changing it requires a rebuild.
- Commands: `m049-start`, `m049-info`, `m049-poll`.
- `m049-poll` is a synchronous poll. It is useful for bring-up and diagnostics; it is not a substitute for a future scheduler-driven event loop.
- Keep a serial console attached during first hardware tests. Do not enable autostart on a production machine until xHCI controller ownership and keyboard availability are confirmed.
