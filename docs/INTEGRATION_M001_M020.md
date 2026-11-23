# PSRE Integrated Build 1.7 — M001 to M020

This cumulative archive extends Build 1.6 (M001-M019) and preserves the previous source tree and root Makefile.

M020 adds SET_CONFIGURATION activation, descriptor-validated SET_INTERFACE selection, interface class identification, and a bounded USB device registry.

CLI additions:
- `m020-configure`
- `m020-alt INTERFACE ALTSETTING`
- `m020-info`

Recommended sequence:
`m014-probe` → `m015-init` → `m016-ports` → `m016-reset PORT` → `m017-enable-slot PORT` → `m018-enumerate PORT` → `m019-config` → `m020-configure` → `m020-info`.

This is not yet a USB class-driver implementation. HID, Mass Storage, Hub and CDC drivers remain future work. Runtime validation is required.
