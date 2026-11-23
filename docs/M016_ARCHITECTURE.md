# M016 Architecture

M016 uses M015's validated controller identity and RUNNING state. It maps the xHCI BAR for the duration of each operation and unmaps it afterward.

## Register layout
- Operational Register base: `CAPLENGTH`.
- PORTSC for port N (1-based): `CAPLENGTH + 0x400 + (N - 1) * 0x10`.
- Port count: `MaxPorts` from M015's validated HCSPARAMS1 snapshot.
- Port Power Control capability: HCCPARAMS1.PPC (bit 3).

## Read-only inventory
For each port, M016 reads PORTSC and reports:
- CCS: current connect status.
- PED: port enabled/disabled.
- OCA: over-current active.
- PP: port power.
- PLS: link state.
- Port Speed ID.
- CSC/PEC/WRC/OCC/PRC/PLC/CEC change flags.

## Explicit reset
1. Validate M015 is RUNNING and the port number is within 1..MaxPorts.
2. Read PORTSC. Require CCS=1, OCA=0 and a supported speed ID.
3. If PPC is advertised and PP=0, set PP and wait 100 ms.
4. Clear only a stale matching completion flag (PRC for USB2, WRC for SuperSpeed).
5. Issue PR for speed IDs 1–3, or WPR for speed IDs 4–5.
6. Poll until reset bit clears and the corresponding completion-change flag is set, with a 5-second timeout.
7. Clear only the matching RW1C flag while preserving PP, PIC and wake-enable control fields.
8. Re-read PORTSC and report connection and PED state.

## Safety
M016 never writes a full PORTSC value back from a raw read. PORTSC includes read/write-one-to-clear (RW1C) change bits; echoing those bits could erase unrelated pending change notifications. Writes are composed only from known control fields and the one intended command/change bit.

## Limitations
This is root-hub port status and reset only. A port becoming enabled does not mean the attached device has been enumerated. No Enable Slot command, Address Device command, descriptor transfer, or USB data transfer exists yet.
