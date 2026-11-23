# PSRE Integrated Build 1.6 — M001 to M019

This archive extends Build 1.5 (M001-M018) and preserves the cumulative project tree and root Makefile.

M019 adds USB Configuration Descriptor retrieval, interface/endpoint parsing, and optional string descriptor reads, using the EP0 control transfer path from M018.

CLI additions:
- `m019-config`
- `m019-info`

Recommended order:
`m014-probe` → `m015-init` → `m016-ports` → `m016-reset PORT` → `m017-enable-slot PORT` → `m018-enumerate PORT` → `m019-config` → `m019-info`.

This remains a single-device, first-configuration, polled xHCI implementation. Runtime validation is still required.
