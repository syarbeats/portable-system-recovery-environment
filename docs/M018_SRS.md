# M018 — USB Addressing and Device Descriptor

## Goal
Continue from M017's Enable Slot result to assign a USB device address through xHCI and read the standard USB Device Descriptor over endpoint zero.

## Preconditions
- M014 has discovered a valid xHCI controller.
- M015 controller is RUNNING with DMA and PCI Bus Master enabled.
- M016 reset has left the target port connected/enabled.
- M017 `m017-enable-slot PORT` completed successfully and retained a non-zero Slot ID.
- Single-threaded CLI operation; no concurrent command/transfer ring producers.

## Implemented
- M015 exposes bounded DMA page allocation, DCBAA slot entry assignment, context-size detection, generic polled event consumption and slot doorbell access.
- M018 allocates four contiguous DMA pages below 4 GiB for Input Context, Device Context, EP0 Transfer Ring and descriptor buffer.
- Supports 32-byte and 64-byte xHCI context formats using HCCPARAMS1.CSZ.
- Builds Slot Context and Control Endpoint 0 Context for one device.
- Issues Address Device Command and checks Command Completion Event.
- Sends GET_DESCRIPTOR(Device) using Setup, Data and Status Stage TRBs on EP0.
- Checks Transfer Event and parses USB version, class/subclass/protocol, EP0 max packet size, VID and PID.
- Uses bounded event waits and keeps DMA pages pinned while controller may own them.

## CLI
- `m018-enumerate PORT` — address the device represented by the successful M017 slot and read its Device Descriptor.
- `m018-info` — show the last M018 result.

## Not implemented
- Configuration Descriptor and interface/endpoint parsing.
- String descriptors (manufacturer/product/serial).
- Updating EP0 max packet size after reading the descriptor.
- Set Configuration, hub support, multiple concurrent device lifecycle, USB HID and Mass Storage BOT/SCSI.
- Safe per-slot teardown/deallocation; M018 DMA allocations remain pinned until controller reset/halt.

## Known limitations
- One active device/slot workflow; rerunning M017 or M018 without resetting/reinitializing xHCI is not supported as a general lifecycle.
- Event polling discards unrelated event types and assumes a serialized CLI and no other ring consumers.
- No SMP locking or interrupt-driven event processing.
- Host compilation does not prove hardware correctness. Validate on QEMU and physical xHCI hardware before relying on this for recovery.
