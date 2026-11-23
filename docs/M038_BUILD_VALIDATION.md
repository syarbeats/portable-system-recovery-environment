# M038 — Build Validation

Host tests stub M034 window geometry/status, M036 app registry, M032 graphics and M033 text rendering. Tests cover multiple containers, scoped widget IDs/state, client coordinate hit-testing, focus routing, callbacks, textbox editing, resize bounds and lifecycle reconciliation.

Host tests and freestanding object compilation do not prove full cross-link, UEFI boot, QEMU runtime, or physical hardware behavior. Those require the x86_64-elf toolchain, NASM, QEMU and a target system.
