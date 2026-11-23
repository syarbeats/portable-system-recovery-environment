# Changelog M046
- Added synchronous USB BOT command executor.
- Added explicit CBW wire serialization, optional data phase, and CSW receive/validation.
- Added SCSI convenience execution and BOT status reporting.
- Added CLI commands for initialization, INQUIRY, TEST UNIT READY and READ CAPACITY(10).
- Explicitly reports BOT reset recovery unavailable until EP0 class-control and endpoint halt-clear support are implemented.
