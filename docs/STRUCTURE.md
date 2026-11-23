# PSRE Integrated M001–M004 Structure

```text
PSRE/
├── Makefile, limine.conf, README.md
├── docs/ (SRS, architecture, test plans for M001–M004)
├── arch/x86_64/ (linker, CPU helpers, M003 GDT/ISR assembly)
└── kernel/
    ├── include/psre/ (memory, Limine, serial, M003, M004 headers)
    └── src/
        ├── main.c, serial.c, limine_requests.c
        ├── memory_map.c, pmm.c, heap.c, vmm.c, memory_diag.c
        ├── subsystem.cpp
        ├── m003/ (CPU, IDT, PIC, PIT, panic, init)
        └── m004/ (PCI, ACPI, init)
```

