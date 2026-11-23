# PSRE M002 Test Plan

## Automated boot-time checks
- Limine memory-map/HHDM response presence.
- PMM initialization.
- Allocate one page, free it, verify success.
- Heap allocate 128 bytes and free it.
- Print diagnostics including free/allocated page counts and CR3.

## QEMU baseline
Use UEFI OVMF and 512 MiB RAM. Attach serial output to stdio. Expected final line: `M002 PASS: memory subsystem baseline initialized.`

## Manual follow-up tests
- Boot with 256 MiB, 512 MiB, 2 GiB and 8 GiB RAM.
- Temporarily add a test for double-free rejection.
- Test heap split/coalesce with multiple allocations and frees.
- Verify a known kernel symbol translates to a physical address.
- Run with QEMU `-d int,cpu_reset` if a fault occurs.

## Safety
Do not run disk write, partition, or recovery operations in M002. No storage driver exists yet.
