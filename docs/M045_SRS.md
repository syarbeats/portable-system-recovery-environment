# M045 — xHCI Bulk Transfer Engine: SRS

## Goal
Provide a polled xHCI Bulk IN/OUT transfer foundation for the MSC BOT transport introduced by M044.

## Functional requirements
- Configure up to four Bulk endpoints for the currently enumerated USB device.
- Validate endpoint address, endpoint number, direction, and maximum packet size.
- Allocate DMA memory for an input context, transfer ring, and data buffer.
- Issue Configure Endpoint through the M015 command ring.
- Submit Normal TRBs and ring the endpoint doorbell.
- Poll Transfer Events with bounded timeout; report actual length from residual bytes.
- Track endpoint and aggregate transfer, timeout, and error counters.

## Constraints
- One currently enumerated device/slot; no multi-device routing.
- Polled operation; interrupts are not enabled by this milestone.
- One outstanding transfer per endpoint.
- Maximum transfer buffer is 4096 bytes.
- Transfer ring has 255 usable entries and does not yet recycle/wrap.
- Event polling shares M015's event queue; unrelated events may be consumed by the existing polling model.
- No BOT CBW/data/CSW transaction orchestration, SCSI execution, USB block device, or FAT32 mount.
