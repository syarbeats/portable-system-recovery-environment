# Build 4.9.11 — smooth mouse
- Non-blocking HID polling (previously up to 10 ms spin per endpoint per tick).
- Drain up to 16 mouse reports per poll; coalesce consecutive moves into one event.
