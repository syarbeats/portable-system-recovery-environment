# M052 Build Validation

Run `make test-m052` for parser/topology/registry host tests and freestanding syntax checks. Run every `tests/m*/run_host_tests.sh` script for cumulative host regression checks.

A successful host test or Makefile dry-run does not establish a successful x86_64-elf link, ISO build, QEMU boot, or physical USB hub operation. Those require the cross-toolchain, image tools, and hardware/runtime testing.
