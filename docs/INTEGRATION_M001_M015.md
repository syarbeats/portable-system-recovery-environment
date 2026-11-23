# PSRE Integration M001–M015

Build 1.2 is cumulative. It retains M001–M014 and adds M015 xHCI controller initialization.

M015 is explicitly invoked from the recovery shell and is not executed automatically at boot. This protects systems where the firmware-owned xHCI controller is still serving USB input devices.

CLI additions:
- `m015-init` — re-probe xHCI via M014 and perform ownership/halt/reset/DMA setup/controller start.
- `m015-info` — report M015 state and DMA safety.

See `M015_SRS.md`, `M015_ARCHITECTURE.md`, `M015_TEST_PLAN.md`, `M015_BUILD_VALIDATION.md`, and `M015_TROUBLESHOOTING.md`.


M016 is included in Build 1.3; see `INTEGRATION_M001_M016.md`.
