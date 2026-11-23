# PSRE Integrated Source M001–M058

This archive is cumulative from the supplied Build 4.9.6 M001–M057 source archive. M058 adds a read-only xHCI/HID/input audit and names the existing M022 Endpoint Context CErr and Interrupt-IN type fields explicitly. All previous milestone source and documentation files are retained.

## Desktop boot preservation
The M058 call is placed after M056 diagnostics. It does not modify M031 framebuffer discovery, M048 desktop startup, M049 USB/GUI coordination, M054 framebuffer handoff, the final `psre_m035_render()` call, or the `psre_m050_run_once()` desktop event loop.

## Input status interpretation
A keyboard endpoint configured in software is not proof of working input. Success requires xHCI Transfer Event completion, M025 report processing, and M030 received/dispatched input events. A timeout with `transfer_in_flight=yes` remains quarantined; M058 does not reset the controller or recycle DMA memory.

## Validation
Run `make test-m058` and the existing milestone host tests. These tests validate source-level behavior only; they do not prove real xHCI transfer completion on QEMU or physical hardware.
