# PSRE Integrated Build 1.8 — M001 to M021

This cumulative archive extends Build 1.7 (M001-M020), retaining the root Makefile and previous milestones.

M021 adds HID interface discovery, HID class descriptor metadata retention, Interrupt IN endpoint validation, GET_DESCRIPTOR(Report), and bounded HID report descriptor parsing.

CLI additions:
- `m021-probe`
- `m021-info`

Recommended sequence:
`m014-probe` → `m015-init` → `m016-ports` → `m016-reset PORT` → `m017-enable-slot PORT` → `m018-enumerate PORT` → `m019-config` → `m020-configure` → `m021-probe` → `m021-info`.

M021 does not yet provide keyboard/mouse input. Configure Endpoint, interrupt transfer rings, transfer event routing, and input event processing remain future work.
