# M014 Troubleshooting

- **No xHCI controller:** run `usb-info`; verify PCI class/subclass/prog-if.
- **No valid BAR:** inspect `xhci-info` and `usb-info`; confirm a memory BAR
  is present and its base is page aligned.
- **MMIO mapping rejected:** M013 may reject a BAR overlapping Limine USABLE
  RAM, an unsupported physical range, an occupied VA slot, or exhausted slots.
- **Capability parser rejected:** confirm the controller returns a valid
  CAPLENGTH, HCIVERSION, HCS1, DBOFF, and RTSOFF snapshot.
- **Mapping count is non-zero:** inspect M013 state; M014 treats an unmap
  failure as a probe failure and reports the retained mapping count.

Do not force-enable PCI bus mastering or write controller registers as a
workaround. M014 is deliberately read-only.
