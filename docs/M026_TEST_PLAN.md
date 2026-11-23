# M026 Test Plan

1. Build-time: strict host GCC syntax compile all C translation units with `-Wall -Wextra -Werror`.
2. Build-time: compile every C translation unit to freestanding x86_64 objects using project flags.
3. Compile the C++ subsystem with project-equivalent freestanding flags.
4. Verify root Makefile dry-run and confirm M022/M025/M026 source inclusion.
5. Static checks: endpoint table capacity is four; each endpoint gets distinct DMA pages; lookup is by endpoint address; keyboard and mouse are initialized independently; polling invokes both active endpoints; legacy CLI aliases remain.
6. Runtime (not performed in this environment): test a composite keyboard/mouse device in QEMU with xHCI; verify key down/up while moving/clicking mouse; test keyboard-only and mouse-only devices; test endpoint configuration failure fallback; inspect queue overflow counters.

## Expected behavior
- Composite HID: `m026-info` reports Keyboard=active and Mouse=active.
- Keyboard-only: keyboard remains active and polling continues.
- Mouse-only: mouse remains active and polling continues.
- Unsupported/non-boot HID: initialization reports no supported input endpoint.

No hardware/QEMU runtime result is claimed by the build-time checks.
