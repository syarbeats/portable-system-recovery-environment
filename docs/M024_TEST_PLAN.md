# M024 Test Plan

1. Enumerate a USB HID boot mouse and complete M019, M020, and M021.
2. Start M025; it selects a boot keyboard first, otherwise a boot mouse.
3. With a mouse-only device, run `m025-poll` and inspect `m025-next`.
4. Verify positive/negative X/Y deltas and button down/up transitions.
5. Negative cases: non-boot mouse, short report, no Interrupt IN endpoint, and timeout.

Wheel reports and physical device tests are outside this milestone's validated scope.
