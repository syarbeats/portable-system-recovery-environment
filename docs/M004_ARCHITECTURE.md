# PSRE M004 Architecture

Kernel main calls `psre_m004_initialize()` after M003 PIT validation. ACPI uses the Limine RSDP response and HHDM to map physical RSDT/XSDT/SDT addresses. PCI uses Configuration Mechanism #1 (CF8/CFC), scanning buses 0–255 and decoding headers and BARs read-only. Both subsystems publish fixed-size inventory/statistics and report through COM1.

No BAR writes, PCI command changes, bus mastering, DMA, APIC programming, ECAM access or driver binding occurs in this milestone. MCFG is inventoried but not used for PCI access.

