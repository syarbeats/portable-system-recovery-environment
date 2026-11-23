# M028 Build Validation

Validation performed for Build 2.3:
- Strict host GCC syntax compilation of all C source files.
- Freestanding object compilation using project-equivalent compiler flags.
- C++ subsystem object compilation.
- Root Makefile dry-run.
- ZIP integrity test.

QEMU, cross-toolchain and physical USB runtime tests must be performed in an environment containing the required tools and hardware. Static compilation does not certify live xHCI behavior.

Additional synthetic parser tests passed:
- No-Report-ID wheel mouse: button bits, signed X/Y and signed vertical wheel.
- Report-ID mouse: eight buttons, signed X/Y, vertical wheel and horizontal pan.
