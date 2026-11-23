# M058 Architecture

M058 is a read-only observer over M015, M018, M021, M022, M025, and M030 status APIs. It does not own DMA buffers or xHCI rings and cannot modify controller state. It prints one compact audit after M056.

The endpoint context encoding in M022 is documented with named constants to avoid confusing the CErr field (bits 2:1) with Endpoint Type (bits 5:3). The encoded value remains CErr=3, Interrupt IN=7.

The current M022 timeout policy quarantines an in-flight TRB: it does not recycle the TRB or DMA buffer after timeout. This is intentionally conservative. A later recovery milestone must first establish controller/endpoint quiescence before reclaiming or reusing that memory.
