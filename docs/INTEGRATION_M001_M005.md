# PSRE Integrated Build 0.2 — Integration Map

All milestones are built from the project root with one Makefile. M005 is initialized after M004 in `kernel/src/main.c`.

| Milestone | Main implementation | Initialization |
|---|---|---|
| M001 | `kernel/src/main.c`, `serial.c`, Limine requests | kernel entry |
| M002 | memory map, PMM, heap, VMM, diagnostics | `psre_kernel_main` |
| M003 | `kernel/src/m003/`, `arch/x86_64/m003/` | `psre_m003_init` |
| M004 | `kernel/src/m004/` | `psre_m004_initialize` |
| M005 | `kernel/src/m005/` | `psre_m005_initialize` |

Build commands: `make clean && make`; boot command: `make run`. The latter requires the ISO toolchain dependencies and QEMU.
