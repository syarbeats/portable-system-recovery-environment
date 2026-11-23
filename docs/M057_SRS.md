# M057 — xHCI HID Transfer Recovery

## Objective
Improve observability and safety of the M022 interrupt-IN path used by M025, while preserving the M048 Desktop boot path and all M001–M056 files.

## Changes
- Count transfer attempts, successful completions, timeouts, completion-code failures, doorbell failures, and unmatched events.
- Record last event type, slot, endpoint, completion code, requested length, actual length, and residual length.
- Keep endpoint-local counters and aggregate counters distinct.
- Mark a TRB as in-flight immediately after ringing the endpoint doorbell.
- On timeout, quarantine the transfer/TRB and its DMA buffer: do not overwrite or recycle memory that may still be owned by xHCI.
- Do not automatically reset xHCI, re-enumerate USB, or change the Desktop startup sequence.

## Safety
A timeout is not proof that the controller stopped reading the TRB or data buffer. M057 therefore prefers a visibly stalled endpoint over unsafe DMA reuse. Recovery requiring controller halt/reset remains a later, explicit operation.

## Limitations
This milestone does not claim that keyboard input is fixed on physical hardware. A successful Transfer Event and valid HID report must still be observed. Mouse support requires a detected mouse interface and is not inferred from a keyboard-only descriptor.
