# Integrated Milestones M001–M009
Build 0.6 integrates M009 into the same PSRE tree, using the existing root Makefile and existing FAT32/CLI translation units. M001–M008 are retained. New API declarations are in `kernel/include/psre/m006/fs.h`; M009 queue implementation is appended to `kernel/src/m006/fat32_fs.c`; commands are registered in `kernel/src/m006/cli.c`.

No separate milestone executable or manual source-copy step is required.
