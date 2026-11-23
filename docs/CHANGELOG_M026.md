# Changelog — M026

- Replaced the singleton M022 Interrupt IN transfer state with a four-entry endpoint table.
- Added endpoint-address-specific configure, transfer, and status APIs.
- Allocated independent DMA Input Context, Transfer Ring and Transfer Buffer pages per endpoint.
- Updated M025 to initialize and poll boot keyboard and boot mouse concurrently.
- Preserved the 64-event FIFO and existing event types.
- Added `m026-start`, `m026-poll`, and `m026-info` CLI aliases.
- Retained all prior milestone sources and the single root Makefile.
