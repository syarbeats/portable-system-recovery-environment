# M007 Architecture

M007 extends the existing FAT32 filesystem layer (`kernel/src/m006/fat32_fs.c`) and serial shell (`kernel/src/m006/cli.c`).

## Path resolution
- Paths beginning with `/` start at the FAT32 root cluster.
- Relative paths start at the current working directory cluster.
- Each intermediate component is looked up in the current directory and must have the directory attribute.
- `.` leaves the cluster unchanged; `..` follows the directory's parent entry, falling back to root if the entry is invalid.
- Path components are 8.3 short names. LFN entries are skipped.

## Operations
- `ls`: enumerates directory cluster chains.
- `cat`: reads file contents and emits printable characters; non-printable bytes are rendered as dots.
- `stat`: reports directory entry metadata.
- `crc32`: streams file bytes through reflected CRC-32/ISO-HDLC.
- Traversal is bounded by the maximum valid data cluster count to avoid unbounded loops on damaged FAT chains.

## Safety
M007 invokes ATA sector reads only. No FAT or data-sector write path is added. Copy-to-disk and repair are intentionally deferred until a separate write-safe storage design, target selection, and confirmation mechanism exist.
