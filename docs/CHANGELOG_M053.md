# CHANGELOG M053

- Added USB 2.0 Hub class driver over active M018 EP0 control transport.
- Added Hub descriptor retrieval and parser integration with M052.
- Added downstream port power, GET_STATUS, reset, and change-feature clear requests.
- Added explicit port scan and CLI status/probe commands.
- Added host mock tests and M053 documentation.
- Limitation: no independent EP0 per slot, hub interrupt endpoint polling, TT routing, or downstream device enumeration.
