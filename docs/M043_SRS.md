# M043 — Recovery Process Manager & Export Progress Engine

## Goal
Provide a bounded cooperative job manager for M008/M009 recovery and serial export, with progress, pause/resume, retry, and a finite job history.

## Functional requirements
- Start a single-file M008 staging job by path.
- Start a multi-file M009 serial export job.
- Advance running jobs only through `psre_m043_tick()`.
- Pause and resume at safe step boundaries.
- Retry a failed job explicitly; no automatic retry loop.
- Track job ID, state, job type, file count, payload bytes, tick count, errors, and the latest 16 history records.
- Emit a recovery manifest before queue export, including per-file CRC32 and SHA-256.
- Keep source FAT32 media read-only.

## Constraints
One active job at a time. M008 file staging remains synchronous within one tick. M009 export emits at most 256 payload bytes per tick (record framing is additional), with 32-byte HEX records. Pause takes effect between ticks. The manager is single-threaded and has no background timer or SMP synchronization.
