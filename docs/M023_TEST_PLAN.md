# M023 Test Plan

1. Enumerate a USB HID boot keyboard and run M019 configuration parsing.
2. Run M020 configuration activation and M021 HID probe.
3. Run `m023-info` before initialization to inspect default state.
4. Initialize through `m025-start` (M025 selects the keyboard and configures its M022 endpoint).
5. Press/release keys and modifiers; inspect `m025-poll`, `m025-next`, and `m023-info`.
6. Verify 8-byte reports, simultaneous key slots, modifier transitions, and rollover error handling.
7. Negative cases: non-boot keyboard, missing Interrupt IN endpoint, short report, and transfer timeout.

No physical keyboard test is claimed by the source compilation results.
