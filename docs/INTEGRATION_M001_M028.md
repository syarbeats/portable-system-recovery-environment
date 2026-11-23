# PSRE Integrated Build 2.3 — M001–M028

M028 is cumulative on Build 2.2 (M001–M027). All earlier milestones remain in this archive.

## Integration changes
- Added `kernel/include/psre/m028/mouse.h` and `kernel/src/m028/mouse.c`.
- M028 retrieves the active mouse interface Report Descriptor using M018 EP0 control-IN.
- M028 parses bounded HID short/long items, Report IDs, per-report input bit offsets, signed logical values and mouse application collections.
- Boot-capable mouse interfaces are switched to Report Protocol with SET_PROTOCOL(1).
- M025 now initializes M028 and configures its Interrupt IN endpoint through M022's independent multi-endpoint API.
- M025 emits mouse movement, button 1–8 transitions, vertical wheel and horizontal pan events.
- Added `m028-info`; `m025-next` displays wheel values.
- Root Makefile includes M028 and identifies Build 2.3.

## Usage
After xHCI startup, USB port reset, device enumeration, configuration activation and `m021-probe`, run `m026-start`. Then use `m028-info`, `m026-poll`, and `m025-next`.

## Compatibility and limits
- M028 currently selects a dedicated mouse interface identified by M021. Combined keyboard+mouse interfaces are excluded to avoid switching the keyboard interface out of Boot Protocol.
- Supports relative X/Y, vertical wheel, Consumer Page horizontal pan and buttons 1–8 when described; absolute pointer axes are ignored.
- Absolute pointer coordinates, >8 buttons, high-resolution wheel scaling, multiple mouse interfaces, composite interfaces and transfer-ring wrap/recovery are not implemented.
- M022 endpoint transfer rings retain the prior finite submission limit.

## Validation
- Strict host GCC syntax and freestanding object compilation: 41/41 C translation units PASS.
- C++ subsystem object compilation: PASS.
- Root Makefile dry-run: PASS.
- Synthetic descriptor/report tests (no Report ID and Report ID with eight buttons/horizontal pan): PASS.
- ZIP integrity: PASS.
- QEMU and physical USB runtime tests: not performed in this validation environment.
