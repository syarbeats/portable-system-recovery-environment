# M038 — Test Plan

1. Initialize the container manager.
2. Create containers for two RUNNING apps with distinct M034 windows.
3. Verify duplicate creation returns the existing association.
4. Add controls and verify per-container IDs/state are isolated.
5. Reject widgets whose rectangles exceed client bounds.
6. Render controls and verify drawing primitives are called.
7. Route mouse hover/click to focused window's container only.
8. Verify Button callback and Checkbox state changes.
9. Verify Textbox printable ASCII and Backspace behavior.
10. Verify focus traversal is per-container.
11. Move a window and verify render uses updated coordinates.
12. Shrink a window and verify out-of-bounds controls are skipped.
13. Stop an app and verify reconciliation removes its container.
14. Compile with strict warnings as errors.
