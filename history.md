# Change History

## 2026-11-19 — Fix M007 `ls` mount failure

- Fixed ATA PIO reads in `kernel/src/m005/storage.c`: select the requested
  device and wait for the device-select delay before polling readiness. This
  prevents status polling against an absent IDE slot left selected by device
  discovery.
- Corrected `wait_data` polling to wait for BSY to clear before interpreting
  ERR/DF status bits.
- Updated `docs/M007_TROUBLESHOOTING.md` with checks for an unmounted volume
  and attaching a FAT32 raw image in QEMU.
- Validation: `make iso` completed, then QEMU booted with a FAT32 test image;
  M006 reported the volume mounted and M007 `ls` listed `HELLO.TXT`.


## 2026-11-19 — M014 live xHCI MMIO capability probe
- Added read-only discovery of the first PCI xHCI memory BAR.
- Mapped the first 0x20 bytes using M013 and captured the first 0x1C bytes.
- Reused M012 parser for validation; temporary MMIO mapping is released.
- Added `m014-info` and `m014-probe` CLI commands and M014 documentation.
- No PCI command writes, bus mastering, controller reset/start, DMA, or USB
  enumeration is performed.


## Build 1.2 — M015 xHCI Controller Initialization
- Explicit CLI-driven xHCI ownership handoff, halt/reset, DMA setup and controller start.
- Allocates DCBAA, optional scratchpad structures, Command Ring, Event Ring and ERST below 4 GiB.
- Configures operational/runtime registers and enables PCI Bus Master only after DMA preparation.
- xHCI interrupts remain disabled; no USB port reset or device enumeration.
- M015 is not invoked automatically during boot.


## Build 1.3 — M016 USB Port Detection & Reset
- Added bounded xHCI root-hub port register mapping and port inventory.
- Reports connection, enable, over-current, power, link state, speed ID and change flags.
- Supports explicit USB2 Port Reset (PR) and SuperSpeed Warm Port Reset (WPR).
- Uses PIT timeouts and avoids echoing unrelated PORTSC RW1C bits.
- Does not enumerate USB devices or transfer data.


## Build 1.5 — M018 USB Address Device and Device Descriptor
- Added DMA page allocation, DCBAA entry, context-size and slot doorbell APIs to M015.
- Added 32/64-byte Input/Device Context construction for one USB device.
- Added Address Device command and polled Command Completion Event handling.
- Added EP0 Setup/Data/Status transfer for GET_DESCRIPTOR(Device).
- Parses USB version, class/subclass/protocol, EP0 max-packet field, VID and PID.
- Configuration descriptors, strings, hubs and class drivers remain future work.
- Static host compilation and Make dry-run pass; no QEMU/physical xHCI runtime validation is claimed.


## Build 2.1 — M026 USB HID Multi-Endpoint Support
- Reworked M022 Interrupt IN transfer state into a four-endpoint table with independent DMA rings and buffers.
- Added endpoint-address-specific configure, transfer and status APIs.
- Updated M025 input manager to initialize and poll boot keyboard and mouse concurrently.
- Preserved the common 64-entry event queue and legacy M025 commands.
- Added `m026-start`, `m026-poll`, and `m026-info`.
- Validation: 39/39 C translation units passed strict host syntax compilation and freestanding object compilation; C++ subsystem compilation passed; root Makefile dry-run passed. No QEMU or physical xHCI runtime validation is claimed.


## Build 2.3 — M028 Advanced USB HID Mouse
- Added descriptor-driven HID Report Protocol mouse handling with report-ID-aware bitfield decoding.
- Added relative X/Y, vertical wheel, horizontal pan and up to eight buttons.
- Integrated M028 with M025/M026 independent endpoint polling and shared input event FIFO.
- Added `m028-info`, `MOUSE_WHEEL` and `MOUSE_HWHEEL` events.
- Validation: 41/41 C units passed strict host compilation and freestanding object compilation; C++ compilation, Make dry-run and two synthetic HID parser tests passed. No QEMU/physical USB runtime validation is claimed.


## Build 2.4 — M029 Keyboard Layout & Character Mapping
- Added US QWERTY mapping for HID usages, Shift/Caps Lock, punctuation, control keys and basic keypad.
- Added optional character field to shared M025 input events and `m029-info`.


## Build 2.7 — M032 Graphics Primitive Engine
- Added software graphics primitives over the M031 framebuffer: pixel, clear, line, rectangle, filled rectangle, circle, filled circle, triangle and filled triangle.
- Added clipped raw RGB24/BGR24 bitmap blitting with stride validation.
- Added explicit `m032-init`, `m032-info`, and `m032-demo` commands.
- No automatic drawing during boot; no alpha blending, font rendering or double buffering.

- Build 3.1: M036 Application Framework — application registry, lifecycle, focused-window event dispatch, state storage and basic controls.

- Build 3.2: M037 interactive GUI widget system (Label, Button, Textbox, Checkbox, focus and event routing).

- Build 3.3: M038 window-based widget containers with per-app/window state, local coordinates, input routing and lifecycle reconciliation.

- Build 3.9: M043 Recovery Process Manager & Export Progress Engine — cooperative M008 staging jobs, incremental M009 serial export, progress, pause/resume, explicit retry/cancel, bounded job history; integrated with M041 and M042 GUI.
