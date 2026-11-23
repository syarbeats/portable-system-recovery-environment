# PSRE Integration M001–M035

Build 3.0 is cumulative from Build 2.9 (M001-M034). M035 adds `kernel/include/psre/m035/shell.h`, `kernel/src/m035/shell.c`, host tests and documentation. The root Makefile compiles M035 and exposes `test-m035`. M034 now exposes a desktop painter hook, called after framebuffer clear and before window rendering.

CLI sequence:
1. `m030-start` — initialize USB HID input router after the USB controller/device/endpoint setup.
2. `m035-start` — initialize M034 and M035 and draw the desktop.
3. `m035-menu` — toggle Start Menu.
4. `m035-poll` — poll M030 once and refresh the desktop clock.
5. `m035-info` — inspect shell state.

M035 is not auto-started during kernel boot.
