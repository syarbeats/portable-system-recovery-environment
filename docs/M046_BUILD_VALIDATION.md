# M046 Build Validation
Host unit tests are run with `make test-m046`. The test uses mocked M045 Bulk transfers and mocked M044 CSW validation; it verifies BOT phase ordering at the API boundary, not physical USB I/O.
Run strict freestanding compilation and the root Makefile dry-run before release. Full cross-link, ISO, QEMU and physical-device testing must be recorded separately and are not implied by host tests.
