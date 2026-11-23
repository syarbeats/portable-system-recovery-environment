# Build 4.9.4.1 — M025 Mouse Fallback Correction

- Fixed M025 endpoint setup to use M024 endpoint address, max packet size and interval when the boot-mouse fallback parser was selected.
- Preserved M028 report-descriptor mouse path.
- Desktop boot and serial recovery flow are unchanged.
- This corrects a source-level bug, but does not establish that a mouse interface was present or that live xHCI transfers work on the target.
- M049 still retains only the first successfully addressed USB device; independent keyboard and mouse devices require a per-slot multi-device refactor.
