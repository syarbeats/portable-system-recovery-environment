# M049 Architecture

Boot sequence when enabled:

1. Validate the M031 framebuffer.
2. Initialize xHCI through M015.
3. Examine ports in ascending order; M016 reset is the connected/over-current guard.
4. M017 Enable Slot, then M018 Address Device and read the Device Descriptor.
5. M019 parse Configuration Descriptor; M020 activate configuration.
6. M021 discover and parse HID Report Descriptors.
7. M025 initialize supported keyboard/mouse endpoints; M030 initialize event routing.
8. M034 initialize window manager; M035 initialize desktop.

The sequence stops after the first successfully enumerated device. Any failure leaves serial recovery operational. `psre_m049_poll(timeout_ticks)` polls M030, drains its FIFO, forwards each event to M034 and M035, and returns whether activity was observed/dispatched.

## Safety boundaries
- Autostart compile-time default is disabled.
- Port count is capped at 64.
- The existing M018 one-attempt-per-controller restriction is respected.
- No writes to storage are added.
- No unbounded event queue drain: the existing M030 FIFO has a fixed capacity.
