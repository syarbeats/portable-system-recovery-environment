# M044 Limitations and Safety

- USB BOT Bulk-Only Transport is not yet executed; M022 has no Bulk transfer API.
- No GET_MAX_LUN, BOT reset recovery, SCSI command dispatch to hardware, USB block device, partition scan, or FAT32 mount from USB.
- Only one currently enumerated USB device lifecycle is supported by the preceding milestones.
- No writes to USB media are performed.
- Next: M045 xHCI Bulk OUT/IN transfer engine, then BOT transaction sequencing and SCSI execution.
