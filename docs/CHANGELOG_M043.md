# Changelog — M043

- Added `psre/m043/process.h` and `kernel/src/m043/process.c`.
- Added single active job state machine: IDLE, RUNNING, PAUSED, COMPLETED, FAILED.
- Added single-file M008 stage jobs and cooperative M009 queue export jobs.
- Added pause/resume, explicit retry, cancel, bounded tick processing, progress counters, and 16-entry circular history.
- Added M009 incremental serial export API with 32-byte HEX records and 256-byte default payload budget per M043 tick.
- Added queue manifest emission before export (CRC32/SHA-256 per file).
- Integrated M041 Recover to RAM and M042 Stage RAM through M043; integrated export and pause/resume/retry controls into M041 Recovery Manager.
- Integrated `m043-tick` into the M035 polling path and added serial CLI controls.
- Added M043 host tests and cumulative Makefile target.
