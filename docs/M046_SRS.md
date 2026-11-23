# M046 — USB Mass Storage BOT Transport: SRS
## Goal
Execute USB Mass Storage Bulk-Only Transport transactions using the M044 MSC discovery/CBW/SCSI layer and M045 xHCI Bulk IN/OUT engine.
## Functional requirements
- Serialize a 31-byte CBW in little-endian wire format.
- Send CBW on Bulk OUT.
- Perform optional data phase using direction from CBW flags.
- Receive a 13-byte CSW on Bulk IN.
- Validate CSW signature, tag, residue and status using M044.
- Track command, byte, failure and recovery state.
- Provide SCSI convenience execution for INQUIRY, TEST UNIT READY, REQUEST SENSE, READ CAPACITY(10), READ(10).
## Constraints
- One synchronous BOT command at a time.
- Maximum payload is constrained by M045 (4096 bytes per transfer).
- Current M045 endpoint transfer ring has no wrap/reuse beyond its initial ring capacity.
- BOT class reset and endpoint CLEAR_FEATURE recovery require EP0 control-transfer support not yet exposed by M018; M046 reports this limitation and never fakes recovery.
- No USB storage block device or FAT32 mount is introduced in this milestone.
