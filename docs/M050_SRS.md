# M050 — PSRE Kernel Event Loop & Device Management

## Goal
Introduce a bounded cooperative scheduler driven by the existing PIT tick, make GUI/input polling progress while the serial recovery shell waits for input, and expose a small device-state manager.

## Functional requirements
- Fixed-capacity task table (8 tasks), periodic callbacks, explicit registration/unregistration.
- Use the existing M003 PIT tick counter; no new IRQ handler or timer hardware ownership.
- Skip stale periodic backlog rather than executing bursts after a delayed callback.
- Poll M049 GUI/input once per tick when M049 reports a ready GUI.
- Discover a GUI that becomes ready after the event loop starts and register its poll task lazily.
- Run M043 process progress every 10 ticks and refresh the M049-derived device snapshot every 100 ticks.
- Keep serial recovery CLI available; CLI input becomes nonblocking and calls the scheduler while waiting, then uses `hlt` until the next interrupt.
- Provide status/report and pause/resume commands.

## Limits / non-goals
- Cooperative, single-core scheduler; callbacks execute synchronously and must remain bounded.
- Not preemptive multitasking and not SMP-safe.
- Device manager reflects the enumerated M049 keyboard/mouse state. It does not yet read xHCI PORTSC change bits, enumerate hubs, detect unplug/replug, or recover disconnected endpoints. True hot-plug requires a later xHCI port-change/event-ring milestone.
- Serial console remains the command interface; this milestone does not create a terminal window in the GUI.
