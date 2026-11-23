# M041 — Architecture

The M035 Recovery Manager app creates its M038 container, then calls
`psre_m041_initialize(container)`. M041 creates a bounded path textbox, status
label, and action buttons. Button callbacks invoke the M006 API surface.

Action flow:
`M035 -> M036 app/window -> M038 container -> M041 callback -> M006 API -> M005/FAT32 or RAM recovery API`.

M008 and M009 implementations are integrated into `kernel/src/m006/fat32_fs.c`
rather than separate source directories. The root Makefile compiles that
translation unit, so M041's API calls are backed by source in the cumulative
archive.

The UI provides status feedback while detailed directory, metadata, checksum,
manifest, queue, and export output is emitted through the serial console.
