# M041 — Test Plan

- Initialize Recovery Manager against a valid M038 container.
- Reject empty and overlong paths.
- Set and retrieve a bounded FAT 8.3 path.
- Verify mount, list, stat, CRC32, recover, queue add/list/export/clear,
  manifest, workspace-info, and single-workspace export dispatch.
- Verify action/success/failure counters.
- Run regression host suites M032–M040 and M035 integration.
- Compile M041 as strict freestanding C and run root Make dry-run.
- Validate cumulative ZIP integrity and SHA-256.

Hardware/QEMU tests are separate and are not implied by host tests.
