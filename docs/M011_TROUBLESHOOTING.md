# M011 — Troubleshooting
- No xHCI controller: ensure PCI inventory exposes a function with class/subclass/PI 0C/03/30.
- DMA resources NOT READY: contiguous RAM below 4 GiB may be unavailable; check PMM free pages and address ceiling.
- `xhci-test` reports unavailable: initialize with an xHCI device present and confirm DMA allocation succeeded.
- `xhci-release` is safe to repeat; it releases only the tracked M011 three-page allocation.
- This milestone does not enumerate USB devices or make a USB flash drive appear as a disk. Do not use `queue-export` as a USB storage export path.
