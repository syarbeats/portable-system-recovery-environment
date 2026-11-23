# M010 — Test Plan
## Static checks
- Host GCC syntax check of `kernel/src/m010/usb.c`, `kernel/src/m004/init.c`, and `kernel/src/m006/cli.c` with `-Wall -Wextra -Werror`.
- Root `make -n all`.
- ZIP integrity.

## Runtime tests (not executed in package build)
1. QEMU with xHCI enabled: verify xHCI controller appears in PCI inventory.
2. QEMU with USB controller disabled: verify clean no-controller report.
3. Physical machine with xHCI: compare BDF, vendor/device IDs and BAR against firmware/OS inventory.
4. Execute `usb-info` repeatedly; verify no controller state changes.
5. Confirm ATA/FAT32 commands remain operational.

No USB device enumeration or USB storage transfer test is claimed.
