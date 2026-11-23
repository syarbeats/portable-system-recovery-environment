# CHANGELOG M051

- Added M051 per-port lifecycle state and event API.
- Added M016 read-only PORTSC accessor; no PORTSC write or RW1C acknowledgement occurs in the accessor.
- Added periodic lifecycle scanning to M050.
- Added `m051-info` and `m051-scan` CLI commands.
- Added M051 SRS, architecture, integration, test plan and validation documents.
- Automatic USB slot teardown/re-enumeration is not included.
