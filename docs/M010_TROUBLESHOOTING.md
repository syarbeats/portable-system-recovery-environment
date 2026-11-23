# M010 — Troubleshooting
- No controller found: ensure the VM or firmware exposes a PCI USB controller; many systems expose xHCI only.
- Unknown interface: PCI Programming Interface does not match common UHCI/OHCI/EHCI/xHCI values.
- BAR is zero or absent: firmware may not assign resources, or device may be disabled.
- This report is inventory only. It does not prove that a USB device is connected or that USB Mass Storage is supported.
