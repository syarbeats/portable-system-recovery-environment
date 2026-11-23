# M040 Integration

- Added `kernel/include/psre/m040/controls.h` and `kernel/src/m040/controls.c`.
- Added M040 source to the cumulative root Makefile and `test-m040` target.
- M035 initializes M040 after M039, dispatches events to M040 before M038/M037, and renders M040 after M038.
- Existing M001–M039 files and APIs are retained.

Example:

```c
psre_m040_control_id_t combo = psre_m040_create(
    container, PSRE_M040_COMBO, "Target disk", 12, 12, 180, 48);
psre_m040_add_item(combo, "Disk 0");
psre_m040_add_item(combo, "Disk 1");
psre_m040_set_value(combo, 0);
```
