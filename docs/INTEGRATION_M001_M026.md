# PSRE Integrated Build 2.1 — M001 through M026

This archive integrates M026 into the cumulative PSRE project. All prior milestones M001–M025 are retained. The root Makefile remains the single build entry point.

## USB HID usage sequence
1. `m015-init` — initialize xHCI.
2. `m016-ports` and `m016-reset PORT` — inspect/reset the connected port.
3. `m018-enumerate PORT` — address the USB device.
4. `m019-config` — read configuration/interface/endpoint descriptors.
5. `m020-configure` — activate the USB configuration.
6. `m021-probe` — discover HID interfaces and report descriptors.
7. `m026-start` — configure available boot keyboard and mouse endpoints.
8. `m026-poll` — poll all active endpoints.
9. `m025-next` — dequeue one input event.
10. `m026-info` — inspect input status.

M026 supports keyboard and mouse concurrently when both boot interfaces are exposed by the same enumerated device. It does not add multi-device support, HID NKRO, mouse wheel, asynchronous input, or safe transfer-ring reuse.
