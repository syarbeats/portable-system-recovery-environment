# CHANGELOG — Build 4.9.3.2

- Fixed incorrect coupling between USB HID auto-enumeration and GUI readiness.
- Fixed unconditional entry into the serial recovery CLI after successful GUI initialization.
- Added explicit final desktop repaint after boot diagnostics.
- Entered the M050 cooperative event loop when M048 reports desktop readiness.
- Kept M006 serial recovery CLI as fallback when GUI initialization fails.
