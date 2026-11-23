# M043 Integration Guide

## CLI
```text
queue-export       start cooperative export of the M009 queue
m043-stage PATH    start one M008 recovery staging job
m043-tick          advance one cooperative step
m043-pause         pause a running job
m043-resume        resume a paused job
m043-retry         retry a failed job
m043-info          show status and retained history
```

For GUI use, open Recovery Manager → Export Queue. The Recovery Manager Recover to RAM action and File Explorer Stage RAM action both enqueue an M043 single-file staging job. Run `m035-poll` repeatedly to advance the export. The Pause / Resume Job and Retry Failed Job buttons control the active M043 job.

`m043-stage PATH` is also available from the serial CLI. A staging tick invokes the existing synchronous M008 read and hash calculation; pause cannot interrupt that read once started.

## Export format
M043 retains M009 `PSRE-MULTI-HEX-EXPORT/1` framing. The manifest is printed first. Host software must capture the serial stream and decode each file's `D:` records. The kernel does not create files on the host automatically.
