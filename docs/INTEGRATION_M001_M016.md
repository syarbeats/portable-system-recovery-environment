# PSRE Integrated Build 1.3 — M001 through M016

Build 1.3 retains M001–M015 and adds M016 USB root-hub port inventory and explicit reset.

M016 is not called automatically during boot. First run `m015-init` to initialize the xHCI controller, then use:
- `m016-ports` — inventory all root-hub ports.
- `m016-reset PORT` — reset one connected port.
- `m016-info` — show the latest M016 operation status.

See the M016 SRS, architecture, test plan, build validation and troubleshooting documents. USB device enumeration and Mass Storage are not implemented in this milestone.
