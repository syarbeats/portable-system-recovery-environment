# PSRE Integration M001–M034
M034 is appended to the Build 2.8 cumulative tree. The root Makefile compiles `kernel/src/m034/window.c`; the serial CLI exposes `m034-init`, `m034-info`, `m034-demo`, and `m034-poll`. M034 depends on M030 input, M031 framebuffer, M032 primitives and M033 fonts. It is explicitly initialized, not auto-started during boot.
