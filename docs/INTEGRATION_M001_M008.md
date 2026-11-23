# Integrated Milestones M001–M008
Build 0.5 preserves the existing single-root-Makefile project and all M001–M007 code. M008 extends `kernel/src/m006/fat32_fs.c`, `kernel/include/psre/m006/fs.h`, and `kernel/src/m006/cli.c`; no separate executable or disconnected project is introduced.

Build commands remain `make`, `make clean`, `make iso`, and `make run`. `make iso` requires Limine release binaries and xorriso. `make run` additionally requires QEMU.
