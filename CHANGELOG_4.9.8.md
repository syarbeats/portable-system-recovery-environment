# Build 4.9.8 — USB input fix
- M049_AUTOSTART default 0 -> 1; USB HID bring-up now runs in a default build.
- M022 idle timeout no longer leaves transfer_in_flight stuck; the TRB stays armed.
- Short Packet completion (code 13) is success (TRBs set ISP).
- Interrupt transfer ring wraps (Link TRB cycle/toggle); previously died after 254 reports.
- Event ring entries routed to the matching endpoint instead of discarded.
- M049 continues scanning ports when a device is not HID.
- Removed duplicate psre_m058_initialize() in main.c.
- Test: tests/m059 (`make test-m059`).
