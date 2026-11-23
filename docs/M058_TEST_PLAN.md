# M058 Test Plan

1. Run `make test-m058`; verify endpoint field encoding and Desktop boot-path guards.
2. Compile `kernel/src/m058/diagnostics.c` with freestanding strict warnings.
3. Run `make -n all` to validate source integration.
4. Boot the generated image on QEMU and physical hardware separately.
5. On hardware, require M022 successful Transfer Events, M025 key/mouse polls, and M030 received/dispatched events before declaring input fixed.

Host checks do not emulate xHCI hardware and cannot prove USB input works.
