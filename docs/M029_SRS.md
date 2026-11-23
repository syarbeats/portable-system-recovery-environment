# M029 — Keyboard Layout & Character Mapping (SRS)

## Goal
Translate USB HID Keyboard/Keypad Usage IDs into PSRE character values so CLI and later GUI layers can consume text input.

## Scope
- US QWERTY layout for letters, number row and common punctuation.
- Shift and Caps Lock interaction for alphabetic keys.
- Shift punctuation mapping.
- Enter, Escape, Backspace, Tab and Space control characters.
- Basic numeric keypad mapping.
- Ctrl+A through Ctrl+Z control-code mapping.
- Character value attached to M025 KEY_DOWN and KEY_REPEAT events.
- `m029-info` diagnostic command.

## Non-goals / limitations
- No Unicode, international layouts, dead keys, compose sequences or IME.
- No AltGr or configurable layout selection.
- Function/navigation keys remain key usages with character value zero.
- Num Lock state does not yet switch keypad navigation behavior.
- Keypad operators are mapped as basic printable characters.
