# M015 Architecture

M015 layers on M014 live xHCI capability discovery, M013 PCI configuration/MMIO mapping, M011's xHCI TRB layout concepts, M002 PMM/HHDM, and M003 PIT timeouts.

## Initialization sequence
1. Require M014 controller/BAR/capability state.
2. Set PCI Memory Space Enable while leaving Bus Master disabled.
3. Map a bounded 64 KiB xHCI register window.
4. Re-read and cross-check CAPLENGTH, HCSPARAMS1, HCSPARAMS2, HCCPARAMS1, DBOFF and RTSOFF.
5. Walk xECP and perform legacy BIOS/OS semaphore handoff if a legacy support capability is found.
6. Clear Run/Stop and wait for HCH; issue HCRST and wait for HCRST and CNR to clear.
7. Allocate DMA memory below 4 GiB:
   - 3 contiguous pages: 256-entry Command Ring, 256-entry Event Ring, one-entry ERST.
   - DCBAA page sized for MaxSlots + 1 entries.
   - Scratchpad pointer array and individual 4 KiB scratchpad buffers if HCSPARAMS2 requires them.
8. Configure DCBAAP, CRCR, CONFIG, interrupter-0 ERSTSZ/ERSTBA/ERDP. IMAN.IE and USBCMD.INTE remain disabled.
9. Enable PCI Bus Master and set Run/Stop.
10. Wait for USBSTS.HCH to clear; report RUNNING.

## DMA constraints
All DMA objects are allocated below 4 GiB for broad compatibility, including controllers without AC64. x86 cache-coherent DMA is assumed. Each object is page aligned; DCBAA, scratchpad pointer array and ERST satisfy their alignment requirements.

## Failure safety
If start may have reached hardware, M015 attempts to halt the controller. DMA memory is freed only after HCH confirms halt. If that cannot be established, DMA allocations are deliberately retained to avoid use-after-free by a bus-mastering controller. The MMIO mapping is released on failed initialization where possible.

## Important limitations
The 64 KiB BAR window is a conservative implementation bound, not a BAR-size discovery mechanism. This milestone does not probe BAR size by writing all ones. The controller is not initialized at boot; hardware takeover requires the explicit `m015-init` command.
