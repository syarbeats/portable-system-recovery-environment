# M015 — xHCI Controller Initialization (SRS)

## Objective
Provide an explicit, bounded initialization path from M014's read-only live xHCI discovery to a running xHCI host controller, without yet enumerating USB devices.

## Functional requirements
- Require a valid M014 live capability probe.
- Enable PCI Memory Space independently; do not enable Bus Master until DMA objects are ready.
- Walk the xHCI Extended Capability chain and perform the Legacy Support BIOS/OS ownership handoff when present.
- Halt and reset the controller with bounded waits.
- Validate 4 KiB page support and capability-register consistency.
- Allocate below-4-GiB DMA structures: DCBAA, optional scratchpad pointer array and scratchpad buffers, Command Ring, Event Ring and ERST.
- Configure DCBAAP, CRCR, CONFIG, ERSTSZ, ERSTBA and ERDP.
- Keep xHCI interrupt generation disabled; this milestone is a polling foundation.
- Enable PCI Bus Master only after DMA structures are initialized, then set Run/Stop and wait for HCH to clear.
- Retain DMA memory if a timeout means the controller may still be bus mastering.

## Non-functional requirements
- No unbounded hardware polling.
- No automatic hardware takeover during boot.
- No disk writes and no USB storage operations.
- Report state, timeout stage, controller identity and DMA safety.

## Explicit invocation
The controller is not initialized automatically. From the PSRE serial CLI:
- `m014-probe`
- `m015-init`
- `m015-info`

## Out of scope
USB port reset, Enable Slot, Address Device, descriptor enumeration, transfer rings, MSI/MSI-X/IRQ handling, USB Mass Storage BOT/SCSI, and filesystem mounting from USB.
