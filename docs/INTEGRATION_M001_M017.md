# Integrated PSRE Build 1.4 — M001 through M017

This archive is cumulative and retains the M001-M016 sources and documentation. M017 adds a polled xHCI command/event path and the Enable Slot operation. Build remains driven by the single root Makefile.

The USB device is not fully enumerated yet: no Address Device or descriptor request is issued. Existing ATA/FAT32 recovery remains read-only.
