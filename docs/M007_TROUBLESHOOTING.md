# M007 Troubleshooting

- `No mounted filesystem`: check `info` for ATA detection, then run `mount`; when using QEMU, attach a FAT32 raw image with `QEMU_DISK=PATH`.
- `Directory not found`: check path spelling and 8.3 short-name format; `ls` first to inspect the parent.
- `Entry not found`: the final path component is not present or is not an 8.3 short name.
- FAT/data read diagnostics: check ATA device, partition bounds, and image integrity.
- A directory entry with a corrupt cluster number is rejected; traversal is bounded to prevent infinite loops.
- LFN display and lookup are not implemented; use the short alias shown in directory entries.
- `..` uses the FAT32 parent directory entry; malformed parent entries fall back to root.
