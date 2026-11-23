# Integrated Milestones M001–M010
Build 0.7 retains the PSRE M001–M009 project and root Makefile. M010 adds `kernel/include/psre/m010/usb.h`, `kernel/src/m010/usb.c`, adds the source to `C_SOURCES`, calls discovery after PCI enumeration, and exposes `usb-info` in the existing CLI.

M010 is deliberately USB controller discovery only. Device enumeration, xHCI operational driver, USB Mass Storage BOT/SCSI, and writing recovered files to a USB drive are follow-on work.
