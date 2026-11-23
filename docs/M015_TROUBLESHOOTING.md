# M015 Troubleshooting

- **M014 probe required**: run `m014-probe`; M015 will not trust a missing/invalid snapshot.
- **MMIO mapping rejected**: inspect BAR type/address and M013 mapping diagnostics. M015 does not size BARs destructively.
- **BIOS handoff timeout**: firmware may retain ownership. M015 stops before controller reset.
- **Controller will not halt**: check USBSTS.HCH and firmware ownership. DMA is not allocated before halt/reset.
- **Reset/CNR timeout**: controller did not complete reset or become ready; inspect hardware/virtual-machine xHCI model.
- **DMA allocation failed**: ensure sufficient usable memory below 4 GiB; scratchpad count can increase allocation requirements.
- **Controller start timeout**: M015 attempts to halt. If halt cannot be confirmed, DMA allocations are intentionally retained.
- **RUNNING but no USB devices**: expected. Port reset, Enable Slot, Address Device and descriptor enumeration are later milestones.
