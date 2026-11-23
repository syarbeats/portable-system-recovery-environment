# PSRE Integrated Build 1.1 — M001 through M014

## Integrated sequence
M001 boot and kernel entry; M002 memory; M003 CPU/interrupts; M004 PCI/ACPI;
M005 ATA discovery; M006 recovery shell; M007 nested FAT32 navigation;
M008 recovery workspace; M009 recovery queue; M010 USB controller inventory;
M011 xHCI DMA ring foundation; M012 capability snapshot parser; M013 PCI
configuration/MMIO infrastructure; M014 live read-only xHCI capability probe.

## M014 call order
M004 initializes PCI inventory, M013 initializes MMIO infrastructure, then
M014 maps the selected xHCI BAR and reads the live capability snapshot. M014
does not require or initialize the M011 DMA rings.

## CLI additions
`m014-info` and `m014-probe`.

## Explicitly not implemented
No xHCI legacy ownership handoff, PCI bus-master enable, DMA programming,
controller halt/reset/start, interrupter setup, port detection/reset, USB
enumeration, BOT/SCSI, or USB Mass Storage.


M015 is added in Build 1.2. See `INTEGRATION_M001_M015.md` for the full cumulative milestone set and explicit controller initialization command.
