# M042 — GUI File Explorer & Recovery Queue Visualization

## Objective
Add a graphical FAT32 file browser and a visual recovery-queue view to PSRE.

## User-facing capabilities
- Launch `PSRE File Explorer` from the M035 Start Menu.
- Browse FAT32 directories using FAT 8.3 names.
- Navigate into directories and move to the parent directory.
- Display file/directory name, type, and byte size in a selectable table.
- Page through the first 64 directory entries cached by the M007 enumeration API.
- Explicitly check the highlighted file before staging it to RAM or adding it to the queue.
- Switch between Files and Queue views.
- View queued file paths and sizes, queue count, and RAM consumption.
- Export or clear the recovery queue.
- Display queue RAM usage using the M040 progress control.

## Limits
- Read-only FAT32 browsing; no disk writes, delete, rename, or copy-to-source.
- FAT 8.3 names only; no LFN.
- M007 Explorer cache: 64 entries per directory; 6 visible entries per page.
- Path input is limited by the M038 textbox capacity to 95 characters.
- M009 queue: 16 files and 128 KiB aggregate RAM.
- Serial HEX export still requires host-side capture/decoding.
- No USB Mass Storage backend is added.
