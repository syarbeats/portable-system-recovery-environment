# M058 — xHCI Endpoint Audit and HID Input Diagnostics

## Purpose
Add read-only diagnostics for the USB HID path after the M001–M057 initialization sequence. M058 records controller, enumerated-device, HID-interface, endpoint-transfer, and M030 router counters to serial output.

## Scope
- Report xHCI running state and the M018 addressed device (slot, speed, VID/PID).
- Report HID interface counts and keyboard/mouse endpoint activation.
- Report M022 transfer attempts, completions, timeouts, completion errors, and unmatched events.
- Report M030 received, dispatched, and dropped input events.
- Make the M022 Endpoint Context field definitions explicit: CErr=3 and Endpoint Type=7 (Interrupt IN).

## Non-goals
- No PCI configuration writes, BAR remapping, xHCI reset, BIOS handoff, USB port reset, device re-enumeration, or DMA release.
- No change to Limine, framebuffer initialization, M048 desktop initialization, desktop rendering, or M050 event-loop handoff.
- No claim of physical keyboard/mouse operation until successful Transfer Events and routed input are observed on target hardware.

## Boot compatibility
M058 is invoked after M056 diagnostics and before device-manager/event-loop setup. It only reads existing status structures and writes serial diagnostics. The existing Desktop startup and event loop remain unchanged.
