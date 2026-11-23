# M048 Architecture

`main.c` calls `psre_m048_initialize()` after memory/PCI/xHCI capability discovery. The coordinator is deliberately non-fatal.

- Framebuffer: M031
- Optional xHCI controller bring-up/port scan: M015/M016 (disabled by default)
- Input readiness gate: M030
- Window manager: M034
- Desktop shell: M035
- Recovery fallback: existing M006 serial CLI

The coordinator does not synthesize input readiness and does not reset USB hardware unless the build explicitly enables xHCI autostart.
