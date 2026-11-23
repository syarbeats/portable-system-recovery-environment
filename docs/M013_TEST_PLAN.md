# M013 Test Plan

## Static/build tests
- Compile modified PCI, VMM, M013, CLI and main units with project freestanding
  warning flags (`-Wall -Wextra -Werror`).
- Run `make -n all` to verify the root build graph includes M013.
- Review that boot initialization never calls the bus-master enable API and
  never maps a live PCI BAR.

## Runtime tests (not claimed unless performed)
1. Boot QEMU and confirm M013 guard self-test PASS.
2. In a dedicated test harness, map a known reserved MMIO range and verify
   virtual-to-physical translation and UC PTE flags.
3. Verify reads/writes only against a documented safe test device register.
4. Unmap and verify translation reports unmapped.
5. Verify RAM-overlap, occupied VA, invalid-size and mapping-capacity rejects.

No live hardware test is part of the current source-only validation.
