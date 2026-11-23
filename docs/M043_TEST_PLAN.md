# M043 Test Plan

- Start cooperative queue export and verify RUNNING state.
- Pause and verify that a tick does not advance the export cursor.
- Resume and verify bounded progress increments.
- Verify completion, byte totals, file totals, and history insertion.
- Start a single-file stage job and verify it completes on one tick.
- Verify a second job is rejected while one is RUNNING or PAUSED.
- Regression: run host suites M032–M043.
- Compile every kernel C source with freestanding strict warnings; compile C++ subsystem; dry-run root Makefile; validate archive integrity.

Hardware-specific serial capture and QEMU/physical boot are outside host-test coverage.
