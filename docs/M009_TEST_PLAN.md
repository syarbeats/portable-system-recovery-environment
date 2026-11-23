# M009 — Test Plan
## Static validation
- Compile FAT32 and CLI translation units with host GCC `-fsyntax-only -Wall -Wextra -Werror`.
- Run root `make -n all`.
- Validate ZIP archive integrity.

## Runtime tests (not executed in this package build)
1. Add one small FAT32 file; inspect `queue-list` and manifest.
2. Add several files, then compare exported payloads with originals.
3. Add duplicate paths; verify entries are separate queue items.
4. Reach 16 entries; 17th addition must be rejected.
5. Reach 128 KiB aggregate limit; next addition must be rejected.
6. Trigger read error during staging; queue count/bytes must remain unchanged.
7. `queue-clear` must reset count and used bytes.
8. Compare disk image before and after; no source sectors may change.
9. Decode each export and verify SHA-256 against manifest.

USB storage tests are deferred until a USB host controller and MSC driver are implemented.
