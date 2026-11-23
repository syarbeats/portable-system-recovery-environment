# M012 Test Plan

## Automated/software tests
1. Build `kernel/src/m012/xhci_caps.c` with the project's freestanding warning
   flags; compilation must be clean.
2. Boot/runtime CLI `xhci-cap-test` must print PASS after all synthetic cases.
3. `xhci-cap-info` must state that live MMIO snapshot is unavailable and
   hardware initialization remains disabled.

## Synthetic positive case
xHCI 1.0, CAPLENGTH 0x40, 8 slots, 4 interrupters, 8 ports, DBOFF 0x1000,
RTSOFF 0x2000 and xECP 0x40 DWORD offset.

## Negative cases
- CAPLENGTH 0x41 (not DWORD aligned).
- Snapshot length 0x18 (truncated).
- Parser API null input/output.
- Future test harness should also cover old version, zero resource counts,
  misaligned offsets and offsets preceding CAPLENGTH.

## Not tested
No full cross-compiler build, QEMU boot, live MMIO access or physical xHCI
controller test is claimed unless separately performed.
