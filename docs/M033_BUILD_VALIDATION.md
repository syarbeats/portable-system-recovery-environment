# M033 — Build Validation

The release validation must report strict C syntax compilation, freestanding object compilation, C++ compilation, root Makefile dry-run, M033 host tests, and ZIP integrity separately. Cross-linking, ISO generation, QEMU boot, and physical display testing are separate runtime gates and must not be inferred from host compilation.
