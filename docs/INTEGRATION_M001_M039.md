# PSRE Integrated Build 3.4 — M001–M039

This archive is cumulative and retains the previous M001–M038 project tree. M039 adds the GUI layout/rendering policy layer over M038 containers. Build remains x86_64 UEFI/Limine, C kernel, C++ subsystem, CLI-first/GUI-capable.

Run `make test-m032` through `make test-m039` for host regression tests. Run `make` with the documented x86_64-elf toolchain for the freestanding kernel; `make iso` additionally requires xorriso and Limine tooling.
