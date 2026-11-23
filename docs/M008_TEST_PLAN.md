# M008 — Test Plan
## Static checks
- Compile `fat32_fs.c` and `cli.c` with the target freestanding flags.
- Confirm root Makefile includes the existing translation unit exactly once.
- Check archive integrity.

## Runtime scenarios (QEMU/physical hardware required)
1. `recover` before mount: expected safe rejection.
2. Recover a zero-byte file: expect valid empty session and SHA-256 of empty input.
3. Recover a small file: compare CRC32 and SHA-256 with host tools.
4. Recover a file larger than 128 KiB: expected rejection.
5. Recover a file whose FAT chain is truncated or unreadable: expected abort, previous session retained.
6. `export`: capture serial output, decode `D:` lines, verify `END:` SHA-256.
7. `clear`: workspace becomes EMPTY and heap free bytes increase.
8. Verify source disk sector image is byte-identical before and after all operations.

Runtime tests have not been executed as part of this package build.
