# M037 — Test Plan
1. Initialize registry and verify empty state.
2. Add four widget types and verify IDs/count.
3. Render and verify M032/M033 calls.
4. Focus button and activate with Enter; verify callback.
5. Hit-test checkbox with M030 pointer coordinates; verify toggle.
6. Set/query checkbox state.
7. Focus textbox, append printable ASCII and delete with Backspace.
8. Test Tab focus traversal.
9. Disable control and verify it cannot receive focus.
10. Validate invalid IDs, null pointers and bounded text.
11. Compile with `-Wall -Wextra -Werror`.
