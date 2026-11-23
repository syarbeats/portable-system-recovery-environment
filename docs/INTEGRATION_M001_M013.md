# PSRE Integrated Build 1.0 — M001 through M013

M013 is integrated into the user-provided `psreV0.12` project base. All prior
source milestones and user modifications are retained. The root Makefile now
includes `kernel/src/m013/m013.c`.

Added:
- `kernel/include/psre/m013/m013.h`
- `kernel/src/m013/m013.c`
- `docs/M013_SRS.md`
- `docs/M013_ARCHITECTURE.md`
- `docs/M013_TEST_PLAN.md`
- `docs/M013_TROUBLESHOOTING.md`

Modified:
- PCI config access API in `kernel/include/psre/m004/pci.h` and
  `kernel/src/m004/pci.c`.
- MMIO mapping/unmapping in `kernel/include/psre/vmm.h` and
  `kernel/src/vmm.c`.
- Boot banner/integration in `kernel/src/main.c`.
- CLI diagnostics in `kernel/src/m006/cli.c`.
- Root `Makefile` and `README.md`.

No PCI device writes or live MMIO accesses are performed automatically.
