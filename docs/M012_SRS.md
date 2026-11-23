# M012 — xHCI Capability Register Model & Safe MMIO Gate

## Objective
Add a freestanding, testable parser for the xHCI capability-register block and
formalize the safety gate required before any controller MMIO access.

## Included
- Little-endian decoding of CAPLENGTH, HCIVERSION, HCSPARAMS1, HCCPARAMS1,
  DBOFF and RTSOFF from a caller-provided byte snapshot.
- Validation of snapshot length, CAPLENGTH alignment/minimum, xHCI version,
  register-offset alignment and non-zero controller resource counts.
- Synthetic-data self-test with positive, misalignment and truncation cases.
- CLI diagnostics: `xhci-cap-test`, `xhci-cap-info`.

## Explicitly not included
No BAR dereference, MMIO page mapping, cache-type setup, PCI command-register
writes, bus-master enable, BIOS/OS ownership handoff, controller reset/start,
CRCR/DCBAAP/ERST programming, doorbells, interrupters, port reset, USB
descriptor enumeration, BOT/SCSI or USB Mass Storage.

## Safety invariant
The parser accepts only a byte buffer already provided by a caller. M012 itself
does not derive a virtual address from the PCI BAR and does not access hardware.
This prevents treating the Limine HHDM mapping as if it automatically covered
PCI MMIO with correct cache attributes.

## Acceptance
- Parser rejects null pointers and snapshots shorter than 0x1C bytes.
- Parser rejects CAPLENGTH < 0x20, non-DWORD-aligned CAPLENGTH, versions below
  xHCI 1.0, misaligned DBOFF/RTSOFF, offsets before CAPLENGTH, and zero resource
  counts.
- Synthetic self-test verifies expected field extraction and malformed inputs.
