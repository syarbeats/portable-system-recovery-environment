# M032 — Test Plan

1. Initialize against a synthetic 32-bit RGB framebuffer and verify no writes during initialization.
2. Verify pixel/rectangle clipping at each edge and rejection outside the surface.
3. Verify horizontal, vertical, diagonal and partially off-screen lines.
4. Verify rectangle outline/fill, including negative origin and clipped extents.
5. Verify circle outline/fill at radius 0, 1 and representative larger radii.
6. Verify filled triangle for both vertex windings, degenerate triangle rejection and off-screen clipping.
7. Verify RGB24/BGR24 bitmap stride and clipping; reject null/short-stride/oversized inputs.
8. Run `m032-demo` under QEMU and on physical UEFI hardware; inspect output visually.

Run `make test-m032` or `tests/m032/run_host_tests.sh` for the synthetic host suite. Host synthetic tests do not substitute for QEMU or physical display validation.
