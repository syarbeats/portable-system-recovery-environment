# M056 Change Log

- Added read-only USB/HID/input diagnostic snapshot.
- Added M022 transfer telemetry: attempts, completion outcomes, doorbell failures, unmatched events and requested/actual/residual lengths.
- Added M056 host tests and Makefile target.
- Preserved M031/M035 Desktop boot and serial fallback.
- No claim of working keyboard/mouse until runtime transfer and input event counters are observed.
