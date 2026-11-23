# M041 — Recovery Manager GUI: Software Requirements

## Goal
Provide a GUI front end for existing read-only FAT32 inspection and RAM recovery APIs.

## Functional scope
- Launch from the M035 Recovery Manager application.
- Accept a FAT 8.3 path through a bounded textbox.
- Mount the first supported FAT32 volume.
- Request directory listing, metadata, and CRC32 through M007 APIs.
- Stage one file through the M008 RAM workspace API.
- Add/list/export/clear the M009 multi-file recovery queue.
- Request workspace and queue manifests, workspace status, and serial HEX export.
- Show operation feedback in the application window; detailed command output remains on serial.

## Safety constraints
- No ATA write operation is introduced.
- Source media remains read-only.
- Paths are bounded to 127 characters and retain the existing FAT 8.3 path rules.
- The GUI does not claim that serial HEX export automatically creates a host-side file.
- USB Mass Storage, GPT, LFN, exFAT, NTFS, and filesystem repair remain out of scope.

## Dependencies
M006 filesystem API, M007 path operations, M008 RAM workspace, M009 queue,
M034 windows, M036 app framework, M038 container, M035 GUI shell.
