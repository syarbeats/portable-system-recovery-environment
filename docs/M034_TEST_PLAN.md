# M034 — Test Plan
1. Reject initialization unless M030 and M031 are ready.
2. Create up to 8 windows; reject the ninth and invalid dimensions.
3. Verify title/text truncation is NUL terminated.
4. Verify focus raises a window and z-order remains unique.
5. Destroy windows and reuse their slots.
6. Verify positions clamp to the framebuffer and sizes reject invalid values.
7. Verify mouse click focus, title drag, close affordance, and mouse release stop dragging.
8. Verify keyboard printable characters and Backspace update focused window text.
9. Verify rendering works with synthetic framebuffer and bounded coordinates.
10. Boot with Limine/QEMU and physical USB keyboard/mouse before claiming runtime support.
