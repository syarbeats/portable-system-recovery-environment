# M053 — USB Hub Class Driver & Downstream Port Control

## Scope
Implement USB 2.0 Hub class control requests over the currently active M018 EP0 transport. Retrieve and validate the Hub descriptor, request downstream port power, read port status/change words, issue port reset, and clear connection/reset change features.

## Requests
- GET_DESCRIPTOR(HUB): bmRequestType 0xA0, bRequest 6, wValue 0x2900.
- SET_FEATURE(PORT_POWER): 0x23 / 3 / feature 8.
- GET_STATUS(PORT): 0xA3 / 0 / four-byte response.
- SET_FEATURE(PORT_RESET): 0x23 / 3 / feature 4.
- CLEAR_FEATURE(C_PORT_CONNECTION/C_PORT_RESET): 0x23 / 1 / feature 16 or 20.

## Limits
Only the currently addressed device represented by M018/M020 can be controlled. M018 owns a single active EP0 context; this milestone does not implement independent per-slot EP0 control, hub interrupt endpoint scheduling, downstream Enable Slot/Address Device, TT routing, or automatic child-device class drivers. Port status scanning is explicit via CLI.
