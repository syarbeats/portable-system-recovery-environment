# M012 Troubleshooting

- `xhci-cap-test` FAIL: verify byte offsets and little-endian test fixture in
  `kernel/src/m012/xhci_caps.c`.
- `xhci-cap-info` says live MMIO unavailable: expected in M012; no MMIO mapping
  API or cache-attribute policy exists in the kernel yet.
- No xHCI controller found: inspect `usb-info`; M012 parser tests do not
  require an actual xHCI controller.
- Do not attempt to cast `bar_address` to a pointer or add the HHDM offset.
  PCI MMIO requires a dedicated mapping with appropriate cache type and
  lifetime management.
