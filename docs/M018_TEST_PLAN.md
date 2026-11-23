# M018 Test Plan

## Static checks
1. Build all C translation units with `-Wall -Wextra -Werror -ffreestanding`.
2. Build the C++ subsystem.
3. Run `make -n all` with the project root Makefile.
4. Verify M018 source is in `C_SOURCES` and CLI commands are registered.
5. Verify the archive with `unzip -t`.

## Runtime checks (not performed by host-only validation)
1. Boot UEFI QEMU with an xHCI controller and a supported USB device model.
2. Run `m014-probe`, `m015-init`, `m016-ports`, and `m016-reset PORT`.
3. Run `m017-enable-slot PORT`; confirm non-zero Slot ID and success completion.
4. Run `m018-enumerate PORT`.
5. Confirm Address Device completion, Transfer Event success, and expected VID/PID from the attached device model.
6. Repeat on physical hardware with serial logging and alternate input, because M015 ownership handoff may disable firmware USB input.

## Failure investigation
- Address Device timeout: inspect command ring, event ring, slot ID, Input Context fields and controller state.
- Transfer timeout: inspect EP0 context, dequeue pointer/cycle bit, setup packet, doorbell target, transfer TRBs and port speed.
- Descriptor parse failure: inspect residual transfer length, completion code and received bytes.

No runtime pass is claimed until these tests have actually been run.
