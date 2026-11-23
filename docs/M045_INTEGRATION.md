# M045 Integration

## CLI sequence
1. Initialize xHCI explicitly with `m015-init`.
2. Reset the target USB port and enumerate/configure the device using the existing M016–M020 sequence.
3. Run `m044-init` to discover an MSC interface.
4. Run `m044-activate` to configure its Bulk OUT and Bulk IN endpoints.
5. Use `m045-info` and `m044-info` to inspect state.

The current M044 command builder and CSW validator remain protocol helpers only. No CLI command sends BOT traffic yet.

## API
- `psre_m045_configure_bulk_endpoint(address, max_packet_size)`
- `psre_m045_bulk_transfer(address, buffer, length, timeout_ticks, actual_length)`
- `psre_m045_get_endpoint_status(address)`
- `psre_m045_get_status()`

Bulk IN copies completed bytes to the caller's buffer. Bulk OUT copies caller data to the DMA buffer before submission.
