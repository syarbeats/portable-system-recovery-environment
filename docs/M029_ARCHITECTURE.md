# M029 Architecture

M029 is a stateless translation layer between HID Usage IDs and ASCII. The caller supplies the current modifier byte and M027 LED mask. Letter case is computed as Shift XOR Caps Lock. Punctuation uses Shift only. Ctrl+letter returns ASCII control codes 1–26.

M025 remains the owner of the event queue. On KEY_DOWN and KEY_REPEAT it calls `psre_m029_translate()` and stores the result in `psre_input_event_t.character`. KEY_UP and non-keyboard events carry character zero. This preserves key usage codes and modifiers for consumers that need physical-key semantics.
