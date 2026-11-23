# M052 Integration

- `psre_m052_initialize()` is called during kernel startup after M051.
- `m052-info` reports the topology/registry counters on the serial CLI.
- Hub and device records are currently populated through the M052 API by a future USB transport integration.
- `psre_m052_process_change_bitmap()` accepts a decoded hub interrupt-IN bitmap; it does not submit or schedule the USB transfer itself.
- Root-hub PORTSC monitoring remains owned by M051. M052 models external hub downstream ports.
