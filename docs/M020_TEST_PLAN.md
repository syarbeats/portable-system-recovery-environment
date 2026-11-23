# M020 Test Plan

## Preconditions
Run M014 probe, M015 initialization, M016 port reset, M017 Enable Slot, M018 Device Descriptor, and M019 configuration parsing successfully.

## Configuration activation
1. Run `m020-configure`.
2. Run `m020-info`.
3. Confirm SET_CONFIGURATION completed and registry entry contains slot, port, VID/PID and configuration value.

## Alternate setting
1. Inspect M019 interface/alternate-setting output.
2. Run `m020-alt INTERFACE ALTSETTING` for a descriptor-listed pair.
3. Confirm SET_INTERFACE completion.
4. Try a nonexistent pair; it must be rejected before sending a request.

## Class identification
Test with devices exposing HID, Mass Storage, Hub, CDC, and vendor-specific interface descriptors. Verify per-class counts.

## Validation caveat
Host compile and Makefile dry-run do not prove USB hardware operation. QEMU and physical xHCI runtime testing remains required.
