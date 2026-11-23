# M045 Architecture

`psre_m045_configure_bulk_endpoint()` obtains the active device context from M018, allocates three DMA pages through M015, clones the current context into an Input Context, sets the Add Context flag for the target DCI, creates a 256-TRB ring with a Link TRB, and submits Configure Endpoint.

Endpoint Context type is Bulk OUT (2) or Bulk IN (6). DCI is `2 * endpoint_number + direction_bit`, where IN uses 1. The transfer ring and data buffer reside in the DMA allocation. `psre_m045_bulk_transfer()` copies OUT payload into DMA memory, submits one Normal TRB with IOC (and ISP for IN), rings the endpoint doorbell, waits for a matching Transfer Event, checks completion code and residual length, and copies IN data to the caller.

M044 owns MSC interface discovery. `psre_m044_activate_transport()` configures the discovered Bulk OUT and Bulk IN endpoints through M045. This does not yet execute BOT commands.
