# M051 Architecture

M050 cooperative scheduler invokes `psre_m051_scan()` every 50 PIT ticks. M051 asks M016 to map the xHCI BAR, read one PORTSC register, and unmap the BAR. It compares CCS against the prior sample and enqueues a fixed-size event. The monitor does not clear PORTSC RW1C bits or issue controller commands.

Flow: PIT -> M050 lifecycle task -> M051 scanner -> M016 read-only PORTSC accessor -> per-port state table -> bounded event FIFO -> CLI/API consumer.
