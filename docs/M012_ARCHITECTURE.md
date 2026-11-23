# M012 Architecture

M012 introduces a pure parser boundary between hardware discovery and future
xHCI MMIO access.

`PCI inventory (M004/M010) -> xHCI identity/BAR metadata (M011) -> [future
MMIO mapping + cache policy] -> capability byte snapshot -> M012 parser`

The bracketed MMIO mapping stage does not exist yet. Therefore current boot
runs a synthetic parser test only. The parser has no dependency on PCI config
space, PMM, HHDM, or the controller's physical BAR.

Register fields decoded:
- CAPLENGTH: byte 0
- HCIVERSION: bytes 2..3
- HCSPARAMS1: bytes 4..7
- HCCPARAMS1: bytes 0x10..0x13
- DBOFF: bytes 0x14..0x17 (low two reserved bits masked)
- RTSOFF: bytes 0x18..0x1B (low five reserved bits masked)

The extended-capability pointer is returned as a DWORD offset from HCCPARAMS1.
No extended-capability chain is followed in this milestone.
