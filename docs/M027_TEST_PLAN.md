# M027 Test Plan

## Static tests
- Compile all C sources with `-std=gnu11 -Wall -Wextra -Werror`.
- Compile all C sources with the project's freestanding flags.
- Compile the C++ subsystem with project flags.
- Run `make -n all` and validate the cumulative ZIP.

## Hardware tests still required
1. Connect a USB Boot Keyboard and initialize xHCI through M026.
2. Press and release ordinary keys; confirm KEY_DOWN/KEY_UP events.
3. Hold a key longer than 500 ms; confirm KEY_REPEAT begins, then repeats every 50 ms.
4. Press Caps Lock, Num Lock, and Scroll Lock; verify keyboard LEDs physically change.
5. Exercise more than six simultaneous keys; confirm ErrorRollOver reports do not corrupt the previous key state.
6. Connect keyboard and mouse together; confirm both endpoints continue to poll.

No QEMU or physical hardware result is claimed by static validation.
