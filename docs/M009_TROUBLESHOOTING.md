# M009 — Troubleshooting
- Queue full: maximum 16 entries; use `queue-clear`.
- RAM capacity exceeded: aggregate file size may not exceed 131072 bytes.
- Source file not found: use FAT 8.3 short names, as in M007/M008.
- Export output missing: capture the full serial session through `PSRE-EXPORT-COMPLETE`.
- Host files are not created automatically; decode each file's `D:` HEX lines and verify its `END:` SHA-256.
- USB flash drive output is not supported in M009. Do not interpret PCI enumeration as USB storage support.
