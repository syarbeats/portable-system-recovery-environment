# M015 Test Plan

## Safe software/build tests
- Compile every C translation unit with warnings as errors.
- Confirm root Makefile includes `kernel/src/m015/xhci_init.c`.
- Confirm M015 is not called from kernel boot initialization.
- Confirm CLI exposes `m015-init` and `m015-info`.
- Inspect failure paths: no DMA free unless controller halt is established after a possible start.

## QEMU tests
- Attach an xHCI controller (`qemu-xhci`) and boot the ISO.
- Check M014 status, then run `m015-init`.
- Confirm controller reaches RUNNING with interrupts disabled.
- Repeat with no xHCI controller; initialization must fail without PCI writes or MMIO mapping.
- Test unsupported/invalid capabilities and timeout paths where practical.

## Hardware tests
- Record controller VID:DID, firmware, BIOS/UEFI settings and serial log.
- Test only on a machine where taking ownership of the xHCI controller is acceptable.
- Verify keyboard/mouse input is not required by PSRE during takeover.
- Do not test USB storage until port enumeration and transfer support exist.
