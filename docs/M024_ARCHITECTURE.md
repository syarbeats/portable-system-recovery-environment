# M024 Architecture

M024 selects a HID boot mouse interface (Subclass 1 / Protocol 2) from M021's parsed interfaces, issues `SET_PROTOCOL(Boot)` through M018 EP0, and decodes three-byte boot reports. X and Y are signed relative deltas; the low three bits of the first byte represent buttons 1–3.

M024 shares the M022 transfer engine. It does not independently allocate xHCI rings or configure an additional endpoint when another endpoint is already configured.
