# PSRE Integrated Milestones M001–M046
Build 4.1 retains the full M001–M045 tree and adds M046 USB BOT transport execution.
M044 discovers MSC BOT interface and constructs CBW/SCSI commands; M045 configures and performs xHCI Bulk transfers; M046 sequences CBW, optional data phase and CSW.
The next required layers are BOT class reset recovery, SCSI sense handling, USB block-device abstraction, and FAT32 access over USB.
