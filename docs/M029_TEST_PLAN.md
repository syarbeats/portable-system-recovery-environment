# M029 Test Plan

Expected mappings for US QWERTY:

| HID usage | Modifiers / LEDs | Expected |
|---|---|---|
| 0x04 (A) | none | `a` |
| 0x04 (A) | Left Shift | `A` |
| 0x04 (A) | Caps Lock LED | `A` |
| 0x04 (A) | Shift + Caps Lock | `a` |
| 0x1E (1) | none | `1` |
| 0x1E (1) | Shift | `!` |
| 0x28 | none | LF (`0x0A`) |
| 0x2A | none | Backspace (`0x08`) |
| 0x2B | none | Tab (`0x09`) |
| 0x2C | none | Space |
| 0x06 (C) | Left Ctrl | ETX (`0x03`) |

Also verify unsupported/function keys return zero and that key usage/modifier fields remain intact in M025 events.
