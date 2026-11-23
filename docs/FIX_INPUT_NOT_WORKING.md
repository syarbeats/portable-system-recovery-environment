# Fix: keyboard and mouse not responding in Desktop GUI

## Root causes
1. **USB never started.** `Makefile` defaulted `M049_AUTOSTART ?= 0`, so `psre_m049_initialize()` returned
   early, M055 reported "No active USB input source" and never polled. Default is now `1`.
2. **Interrupt endpoint wedged after first idle poll** (`m022/interrupt.c`). HID interrupt-IN endpoints NAK
   until a key/mouse change, so a 1-tick poll *normally* times out. The old code left `transfer_in_flight=true`
   on timeout and refused every later poll, so the endpoint died permanently. The TRB now stays armed and is
   collected on a later poll.
3. **Short Packet treated as error.** TRBs set ISP, so reports shorter than max-packet complete with code 13;
   only code 1 was accepted. Both are now success.
4. **Ring never wrapped.** After 254 reports (seconds of mouse movement) polling stopped. Link TRB cycle/toggle
   handling added.
5. **Cross-endpoint event loss.** Events for the mouse were discarded while waiting on the keyboard. Events are
   now routed by (slot, DCI) to per-endpoint buffers.
6. M049 gave up if the first connected device was not HID; it now tries the next port.
7. `psre_m058_initialize()` was called twice in `main.c`.

## Round 2 (QEMU log: only keyboard 0627:0001 found, mouse never started)
8. **Multi-device USB.** M018 and M020 refused a second enumeration ("one attempt per xHCI init"). They now reset
   per device. M049 scans all ports, calls the new `psre_m025_add_device()` for each HID device while its
   descriptors are still current, and stops when keyboard AND mouse are active.
9. **Endpoint 0x81 collision.** QEMU usb-kbd and usb-mouse both use 0x81; M022 looked endpoints up by address
   only, so the mouse would have been silently mapped to the keyboard's endpoint. Endpoints are now keyed by
   (slot, address) (`psre_m022_interrupt_in_slot`).
10. Slot Context "Context Entries" is now raised to the new endpoint's DCI (needed on real xHCI hardware).
11. `make run` now adds `-device qemu-xhci -device usb-kbd -device usb-mouse` (override with `QEMU_USB=`).
    Use usb-mouse, NOT usb-tablet (absolute pointer is not supported by the mouse parser).

## Known limits (not changed)
- Devices behind a USB hub are not enumerated (M052/M053 transport is not integrated).
- Halted endpoints (STALL / transaction error) are counted but not recovered with Reset Endpoint.
- Not validated on QEMU or real hardware; no cross-toolchain available here. Verify with serial log:
  `[M026] Keyboard and mouse endpoints active.` and M058 `transfers completed` increasing.

## Test
`make test-m059` (simulated xHCI: idle timeout, short packet, 700 reports across ring wrap).
