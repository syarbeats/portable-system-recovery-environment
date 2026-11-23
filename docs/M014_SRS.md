# M014 — Live xHCI MMIO Discovery and Capability Validation

## Goal
Use the PCI and MMIO infrastructure from M013 to perform a bounded, read-only
probe of the first discovered PCI xHCI controller and validate its live
Capability Register snapshot with the M012 parser.

## Functional requirements
- Locate PCI class `0x0C`, subclass `0x03`, programming interface `0x30`.
- Select a non-zero memory BAR from the discovered PCI function.
- Map only the first `0x20` bytes through `vmm_map_mmio`.
- Read only the first `0x1C` bytes (CAPLENGTH through RTSOFF).
- Reuse `psre_m012_parse_capability_snapshot` for validation.
- Unmap the temporary MMIO mapping on every completed read path.
- Report PCI BDF, BAR, xHCI version, CAPLENGTH, slots, interrupters, ports,
  DBOFF, RTSOFF, and xECP DWORD offset.

## Safety constraints
- No BAR sizing probe (no all-ones writes).
- No PCI command-register changes; PCI bus mastering remains disabled unless
  another explicitly authorized subsystem changes it.
- No controller register writes, legacy BIOS handoff, halt, reset, start,
  doorbell, interrupt setup, port reset, or USB enumeration.
- The module does not infer the BAR's full aperture size. It dereferences only
  the first 28 bytes; platform firmware/device configuration must provide a
  valid xHCI MMIO BAR.
- M013 rejects mappings overlapping Limine `USABLE` memory. Other memory-map
  types and PAT configuration remain governed by M013's documented limits.

## CLI
- `m014-info`: display the last probe result.
- `m014-probe`: repeat the read-only probe.

## Completion criteria
A valid xHCI BAR is mapped, the live snapshot passes M012 validation, and the
temporary mapping is released. If no controller exists or mapping/parser
validation fails, PSRE reports the failure without attempting controller
initialization.
