# M026 Architecture

M026 retains the M022 xHCI Configure Endpoint and Normal TRB flow but replaces its singleton transfer-ring state with a fixed table of four endpoint contexts. Each table entry owns three DMA pages: Input Context, Transfer Ring, and Transfer Buffer. The endpoint address maps to the xHCI DCI (`2 * endpoint_number + 1` for Interrupt IN).

`psre_m022_configure_interrupt_in_multi()` configures one endpoint and records its DMA resources. `psre_m022_interrupt_in_multi()` selects that endpoint's ring, rings its doorbell, waits for its matching Transfer Event, copies completed bytes, and advances only that endpoint's enqueue index.

M025 now initializes M023 keyboard and M024 mouse independently. It configures both discovered Interrupt IN endpoints and retains whichever endpoint successfully configures. A poll cycle services keyboard then mouse, decodes each report through the existing boot-protocol parser, and publishes events to the common FIFO.

## Ownership and limitations
The controller owns DMA structures after Configure Endpoint. PSRE does not free them. Polls are serialized; only one transfer is outstanding at a time. The implementation supports one addressed USB device and a maximum of four endpoints. The ring deliberately stops before wrap to avoid unsafe live Link TRB updates.
