# PSRE Integrated Build 4.9.6 — M001–M057

Cumulative source package based on Build 4.9.5 (M001–M056). M057 updates the M022 interrupt-IN transfer path and adds endpoint-level diagnostics.

## Desktop preservation
No changes were made to `kernel/src/main.c`, M031 framebuffer initialization, M048 Desktop initialization, or the M050 event-loop startup order. Desktop boot remains the existing Build 4.9.5 path.

## USB input
M022 now records attempts, success, timeout, completion errors, doorbell failures, unmatched events, and last-event metadata. After a timeout, the submitted TRB and associated DMA buffer are marked in-flight and are not reused. This avoids unsafe DMA buffer overwrite, but does not by itself recover a stalled controller.

## Build
- `make test-m057`
- `make test-m056`
- `make test-m055`
- `make all`
- `make iso`

Set `M049_AUTOSTART=1` only when intentionally using the same USB controller takeover configuration as the previous successful test.

## Validation boundary
Host tests, strict syntax checks, and Make dry-run are not equivalent to a successful cross-link, ISO boot, QEMU test, or physical USB keyboard/mouse test.
