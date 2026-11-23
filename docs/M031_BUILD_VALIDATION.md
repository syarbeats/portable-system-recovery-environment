# M031 — Build Validation

Required checks: strict C compilation (`-Wall -Wextra -Werror`), freestanding object compilation, C++ subsystem compilation, Makefile dry-run, and ZIP integrity. Full cross-link, ISO generation, QEMU boot, and physical display tests are tracked separately and must not be inferred from host compilation.


## Build 4.9.2 framebuffer recovery update
- M031 now records a specific validation error instead of returning an opaque failure.
- Diagnostic output includes Limine response presence, response revision/count, and first descriptor geometry when safe to inspect.
- 16/24/32-bpp RGB framebuffers are accepted when masks and pitch validate.
- Startup writes a temporary RGB test pattern before the graphical boot log redraws the display.
- Limine entry requests 1024x768 to improve GOP mode selection consistency.
- Physical UEFI/QEMU display validation remains required.
