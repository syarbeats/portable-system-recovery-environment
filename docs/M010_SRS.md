# M010 — USB Host Controller Discovery
## Objective
Integrate USB host-controller discovery into PSRE's existing PCI inventory without performing unsafe controller MMIO writes.

## Requirements
- Identify PCI Base Class 0x0C, Subclass 0x03 devices.
- Classify Programming Interface 0x00 UHCI, 0x10 OHCI, 0x20 EHCI, 0x30 xHCI.
- Report PCI BDF, vendor/device IDs, command register and BAR type/address.
- Run automatically after PCI enumeration and expose `usb-info` in the recovery CLI.
- Preserve all M001–M009 functionality and root Makefile.

## Safety / scope
This milestone is discovery-only. It does not enable PCI bus mastering, change PCI command bits, map/program controller MMIO, reset controllers, enumerate USB devices, or issue USB storage commands. USB Mass Storage is not yet available.
