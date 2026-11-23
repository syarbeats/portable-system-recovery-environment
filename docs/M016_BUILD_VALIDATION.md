# M016 Build & Validation

## Static validation
- All C sources compiled with host GCC using freestanding project flags, `-Wall -Wextra -Werror`.
- C++ subsystem compile checked independently.
- Root Makefile dry-run checked.
- ZIP integrity checked after archive creation.

## Runtime validation required
The current build environment does not include the PSRE x86_64 cross compiler, NASM or QEMU. Therefore a full kernel link, ISO creation, QEMU port reset, and physical xHCI reset have not been verified. Host compilation is not evidence that the controller accepts the reset sequence.
