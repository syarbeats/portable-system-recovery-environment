# M042 — Integration

- Added `kernel/include/psre/m042/file_explorer.h` and
  `kernel/src/m042/file_explorer.c`.
- Added structured directory-entry enumeration to M006/M007.
- Added read-only M009 queue metadata inspection APIs.
- Extended M040 Table with row selection and consume-once selection reporting.
- Added `PSRE File Explorer` as the fifth M035 built-in application and Start Menu item.
- M035 routes input to M040 and M042 before legacy widget dispatch.
- Added M042 host tests and root Makefile target `test-m042`.
- All previous milestone files are retained in the cumulative source archive.
