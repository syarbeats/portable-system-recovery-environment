# PSRE Integrated Build M001–M052

This build preserves the cumulative M001–M051 source and adds M052's USB hub descriptor parser, topology registry, downstream port change bitmap decoder, multi-device registry, event FIFO, startup initialization, and `m052-info` serial command.

M052 is a topology/registry foundation, not a complete hub class driver. The current xHCI and USB enumeration path remains effectively single-device. Real hub power control, interrupt endpoint scheduling, downstream slot/address/configuration, transaction translators, and reconnect recovery remain future work.
