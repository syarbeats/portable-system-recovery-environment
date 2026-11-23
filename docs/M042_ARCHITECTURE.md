# M042 — Architecture

M035 registers a fifth built-in application, `PSRE File Explorer`. Launching it
creates an M036 app/window and an M038 window container. M042 builds the path
textbox, selection checkbox, navigation/action buttons, and status label in
that container. M040 supplies a selectable three-column table and queue-RAM
progress bar.

The filesystem data path is:
`M042 -> M007 structured directory enumeration -> M006 FAT32 read path -> M005 ATA PIO`.

The queue data path is:
`M042 -> M009 queue inspection/add/export/clear APIs -> static RAM recovery pool`.

M006 now exposes a bounded `psre_m007_read_dir()` API returning short name,
size, first cluster, and attributes. It skips deleted entries, LFN entries,
volume labels, and dot entries, matching existing M007 listing behavior.

M009 now exposes read-only queue count/usage and per-entry metadata. It does not
expose the queue's RAM payload to the GUI.

M040 Table selection is event-based and consumable once, preventing stale row
selection from being interpreted as a new click. M042 tracks the selected
directory entry and requires the checkbox before staging or queueing a file.
