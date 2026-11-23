# PSRE Integration M001–M032

M032 adds `kernel/include/psre/m032/graphics.h` and `kernel/src/m032/graphics.c`, registers the source in the single root Makefile, updates the build identity, and adds `m032-init`, `m032-info` / `m032-demo` serial CLI commands. M031 remains the framebuffer provider. No M032 drawing is triggered automatically during boot.
