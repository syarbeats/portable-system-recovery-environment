# Changelog — M052

- Added bounded USB 2.0 hub descriptor parser (descriptor type 0x29).
- Added hub topology registry with parent hub/port and depth validation.
- Added hub interrupt bitmap decoder and downstream port-change events.
- Added bounded multi-device registry and add/remove lifecycle events.
- Added M052 startup initialization and `m052-info` serial command.
- Added M052 host tests and SRS, architecture, test, integration, and validation documents.
- No claim of physical hub enumeration; USB transport operations remain unimplemented.
