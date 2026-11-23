# M025 Test Plan

## Keyboard
1. Connect a supported boot keyboard.
2. Complete M015–M021 setup.
3. Run `m025-start`.
4. Repeatedly run `m025-poll`, then `m025-next`.
5. Verify key-down/up and modifier transitions.

## Mouse
1. Use a mouse-only boot HID device.
2. Complete M015–M021 setup and run `m025-start`.
3. Poll and dequeue events.
4. Verify signed X/Y deltas and button down/up transitions.

## Queue
- Generate more than 64 events without dequeuing; verify dropped counter increments.
- Drain queue; verify FIFO order and count returns to zero.
- Disconnect or stop device; verify transfer errors are counted.

QEMU and physical USB runtime tests remain required.
