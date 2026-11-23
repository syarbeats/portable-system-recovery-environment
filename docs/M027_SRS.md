# M027 — Advanced USB Keyboard: Software Requirements

## Goal
Extend the M023 Boot Keyboard driver and M025 input path with robust key state, repeat generation, and keyboard lock LED control while preserving M026 multi-endpoint operation.

## Requirements
1. Maintain a 256-usage bitmap for keyboard usages 0x00–0xFF.
2. Reject HID ErrorRollOver reports (usage values 1–3) without changing the prior key state.
3. Generate repeat events for the most recently pressed non-modifier key after 50 PIT ticks, then every 5 ticks.
4. Generate repeat as `PSRE_INPUT_KEY_REPEAT` through the existing M025 FIFO.
5. Toggle Caps Lock, Num Lock, and Scroll Lock LED bits on corresponding key-down transitions.
6. Send the one-byte HID Output Report with SET_REPORT over EP0.
7. Preserve the existing M023 Boot Protocol selection and M026 keyboard+mouse endpoint polling.

## Explicit limitations
- This milestone remains Boot Protocol and is 6KRO, not report-protocol NKRO.
- Repeat timing assumes PIT runs at 100 Hz (50 ticks initial delay, 5 ticks repeat interval).
- One most-recently-pressed key repeats; no configurable per-key repeat policy.
- LED control requires a keyboard supporting the standard one-byte Boot Keyboard LED output report.
- No physical keyboard validation has been performed.
