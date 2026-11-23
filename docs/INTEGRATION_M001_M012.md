# PSRE Integrated Build 0.9 — M001 through M012

M012 is integrated into the same project tree and root Makefile. It adds the
xHCI capability snapshot parser, synthetic tests, CLI diagnostics and boot
self-test. All M001-M011 source remains present.

## Added files
- `kernel/include/psre/m012/xhci_caps.h`
- `kernel/src/m012/xhci_caps.c`
- `docs/M012_SRS.md`
- `docs/M012_ARCHITECTURE.md`
- `docs/M012_TEST_PLAN.md`
- `docs/M012_TROUBLESHOOTING.md`

## Commands
- `xhci-cap-test`
- `xhci-cap-info`

## Safety boundary
This build still does not operate the xHCI controller. No MMIO BAR is accessed,
no PCI bus mastering is enabled, and no USB devices are enumerated. M012
parses synthetic register bytes to validate the decoding logic only.
