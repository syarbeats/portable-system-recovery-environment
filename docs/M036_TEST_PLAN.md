# M036 Test Plan

1. Initialize framework; verify empty registry and ready status.
2. Register one app; verify stable ID and idempotent registration for the same descriptor title pointer.
3. Reject invalid app IDs and launches before registration.
4. Launch app; verify RUNNING state, window association and initial text.
5. Dispatch keyboard event with app window focused; verify callback and state mutation.
6. Stop app; verify window destruction and STOPPED state.
7. Relaunch stopped app; verify counters and RUNNING state.
8. Close an M034 window outside framework and call reconcile; verify STOPPED transition.
9. Run M035 shell tests to verify built-in app registration and launcher integration.
10. Compile all C translation units with `-Wall -Wextra -Werror` and freestanding project flags.

Hardware/QEMU interaction tests are separate and must not be inferred from host tests.
