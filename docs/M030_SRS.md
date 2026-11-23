# M030 — Kernel Input Subsystem: Software Requirements Specification

## Goal
Provide a stable kernel-facing input API above the USB HID-specific M025 layer.
Applications and future GUI components must consume normalized events and
query current input state without depending directly on xHCI/HID drivers.

## Functional requirements
1. Initialize only after M025 reports initialized.
2. Poll M025 and route all available events into an independent 128-entry FIFO.
3. Support keyboard down/up/repeat, mouse movement, mouse button transitions,
   vertical wheel and horizontal wheel events.
4. Maintain a 256-usage HID keyboard bitmap, modifier mask, mouse button mask,
   and accumulated relative mouse X/Y coordinates.
5. Support up to eight event subscribers, each filtering one event type or all.
6. Support subscriber removal and event dequeue.
7. Expose status counters and read-only state queries.
8. Saturate mouse coordinates at signed 32-bit limits rather than overflow.

## Non-functional requirements
- Freestanding C11-compatible implementation; no dynamic allocation.
- Bounded memory and deterministic queue operations.
- No direct hardware register access in M030.
- Callbacks execute synchronously in the caller's context.

## Limits
Single CPU only; no SMP synchronization. Polling is explicit. Queue overflow
increments a drop counter. Subscriber callbacks must not block. Key state is
indexed by HID Usage ID, not Unicode characters. Mouse coordinates are relative
accumulators, not screen coordinates.
