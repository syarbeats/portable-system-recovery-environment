# M016 — USB Port Detection & Reset (SRS)

## Objective
Detect xHCI root-hub ports and provide a guarded, explicit port reset operation after M015 reports the controller in RUNNING state.

## Functional requirements
- Require M015 state RUNNING, PCI Bus Master enabled, and a discovered controller.
- Map only the bounded MMIO window required for the Operational Registers and PORTSC array.
- Read each port's CCS, PED, OCA, PP, PLS, speed ID and RW1C change flags.
- Report speed IDs 1–5 as Full, Low, High, SuperSpeed and SuperSpeedPlus.
- Respect the HCCPARAMS1 Port Power Control (PPC) capability.
- Reject reset requests for invalid port numbers, disconnected ports, over-current ports, and unsupported speed IDs.
- Use PR for USB 2.x speed IDs and WPR for SuperSpeed speed IDs.
- Poll reset completion with bounded PIT-based timeout.
- Clear only the relevant RW1C completion flag, preserving port control fields and avoiding read-back of unrelated change bits.
- Verify that the device remains connected and report whether PED is set.

## CLI
- `m016-info`
- `m016-ports`
- `m016-reset PORT`

## Out of scope
Enable Slot, Address Device, input/output contexts, descriptor requests, transfer-ring scheduling, USB keyboard/mouse drivers, USB Mass Storage BOT/SCSI, and filesystem mounting from USB.
