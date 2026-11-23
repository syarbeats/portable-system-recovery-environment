# PSRE Integrated Build 0.4 — M001 to M007

M007 is integrated into the same source tree and Makefile as M001-M006. No separate milestone copy or manual source merge is required.

The build remains rooted at this directory:
```sh
make clean
make
make run
```

M007 extends M006's read-only FAT32 layer and serial CLI. It adds path navigation, nested directory listing, path-aware file display, metadata inspection and CRC32. All previous milestones remain present.

See `M007_SRS.md`, `M007_ARCHITECTURE.md`, `M007_TEST_PLAN.md`, and `M007_TROUBLESHOOTING.md`.
