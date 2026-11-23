# M017 — USB Enumeration Stage 1: Enable Slot

## Goal
Extend the live xHCI controller from M015/M016 with a first hardware command and polled Command Completion Event handling.

## Scope implemented
- Reuses the command ring, event ring, ERST and MMIO mapping established by M015.
- Adds a bounded polled command submission API in M015 for later milestones.
- Validates M015 RUNNING state and PCI Bus Master Enable.
- Validates that the selected port is connected and enabled (CCS=1, PED=1).
- Issues xHCI Enable Slot Command (TRB type 9) through command doorbell 0.
- Polls the event ring for Command Completion Event (TRB type 33).
- Checks Completion Code = Success (1) and non-zero Slot ID.
- Uses bounded PIT timeout and reports the completion result.

## CLI
1. `m014-probe`
2. `m015-init` (explicitly takes xHCI ownership; may disable USB keyboard/mouse)
3. `m016-ports`
4. `m016-reset PORT`
5. `m017-enable-slot PORT`
6. `m017-info`

## Not implemented
M017 does not yet issue Address Device, allocate per-slot Device/Input Contexts, create endpoint-0 transfer rings, send Setup/Data/Status Stage TRBs, read Device/Configuration/String descriptors, or support hubs/multiple device lifecycle. A successful result means only that xHCI allocated a slot; it is not full USB device enumeration and does not identify VID/PID.

## Safety and limitations
- Hardware operation is explicit; no automatic xHCI initialization or slot allocation at boot.
- Single-controller, serialized CLI operation; no SMP locking or interrupt-driven event processing.
- Command/event ring producer-consumer state is software-maintained and assumes no other command producer.
- No runtime validation on QEMU or physical hardware is claimed by the host compile checks.
- If a command times out, controller/ring state may require a controller reset before retry. Do not assume a timed-out command was cancelled.
