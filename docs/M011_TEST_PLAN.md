# M011 — Test Plan
## Static checks executed
- Host GCC syntax check with `-Wall -Wextra -Werror` for `xhci.c`, M010 `usb.c`, M004 init, CLI, PMM, and FAT32 subsystem.
- Root `make -n all`.
- ZIP integrity.

## Runtime tests required (not executed in this package build)
1. QEMU xHCI-present boot: verify selected PCI BDF and allocated DMA base below 4 GiB.
2. Run `xhci-test`; verify Link TRB, No-Op TRB type/cycle, ERST base and segment size.
3. Run `xhci-release`; verify PMM free-page count returns to its prior value.
4. Re-run `xhci-info`; verify DMA resources show NOT READY.
5. Boot without xHCI: verify no-controller message and no DMA allocation.
6. Stress repeated allocate/self-test/release cycles.
7. Confirm M001–M010 CLI and ATA/FAT32 functions remain available.

## Hardware validation boundary
No xHCI controller start, USB port reset, device enumeration, or USB transfer has been tested or implemented in M011.
