# M008 — Troubleshooting
- `RAM allocation failed`: run `clear` to release the prior staged file, or use a smaller file.
- `File exceeds 128 KiB`: current milestone intentionally limits a single staged file to 131072 bytes.
- `Source file not found`: use a FAT 8.3 short-name path (LFN is not implemented).
- Export output appears incomplete: capture the complete serial session including the `END:` record; check terminal logging settings.
- Host file is not created automatically: parse the `D:` lines, hex-decode them in order, then verify against `END:` SHA-256.
- Do not use this milestone as a forensic write blocker or certified recovery tool; hardware write protection is outside this milestone.
