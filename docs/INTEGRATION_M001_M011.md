# Integrated Milestones M001–M011
Build 0.8 retains all previous PSRE milestones in the same tree and root Makefile.

M011 changes:
- Adds `kernel/include/psre/m011/xhci.h` and `kernel/src/m011/xhci.c`.
- Extends PMM with contiguous physical page allocation/free.
- Calls M011 after M010 PCI USB controller discovery.
- Adds `xhci-info`, `xhci-test`, and `xhci-release` to the existing CLI.
- Adds M011 design, SRS, test and troubleshooting documentation.

The M011 implementation is a safe xHCI driver foundation, not a complete operational xHCI host controller driver. USB device enumeration and Mass Storage remain future work.
