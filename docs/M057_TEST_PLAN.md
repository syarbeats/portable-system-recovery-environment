# M057 Test Plan

1. `make test-m057`, `make test-m056`, and `make test-m055`.
2. Strict freestanding syntax-check all C sources.
3. Build with the existing cross toolchain and create ISO.
4. Boot with the same QEMU/physical configuration used for Build 4.9.5.
5. Confirm the framebuffer Desktop still appears.
6. Inspect M022/M057: attempts should increment; success, timeout, completion errors, doorbell errors, and unmatched events must be separately reported.
7. If timeout occurs, confirm `in-flight=yes` and that subsequent polling does not resubmit/overwrite the same TRB.
8. If Transfer Event success occurs, confirm M025 key polls and M030 received/callback counters increase when keys are pressed.
9. Test a separate USB mouse only after its HID interface is enumerated.

Host tests and syntax checks do not establish xHCI hardware correctness.
