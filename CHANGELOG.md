
## Build 4.9.15 — terminal scroll bar
- M061: real scroll bar on the right edge of the terminal: up/down arrow buttons, draggable thumb sized to the
  visible fraction of the 256-line history, click on the track to page up/down. Mouse wheel, PgUp/PgDn unchanged.
- Scroll range is exact (cannot scroll past the oldest line), typing returns to the bottom.
- `psre_m061_get_scrollbar()` exposes the geometry (used by tests).

## Build 4.9.14 — larger desktop text
- M033: UI "large" mode. Every PSRE_M033_FONT_8X8 request is drawn with the 8x16 glyphs cropped to 13 rows
  (capital letters ~9 px instead of ~6 px). Character advance is unchanged, so horizontal layouts keep working.
  Line pitch 14 px (old 8 px lines overlapped).
- M035 enables it when the desktop starts; `m033-font [normal|large]` switches at runtime (also from the GUI terminal).
- Terminal uses 15 px rows in large mode (10 px in normal).
- Fixed clipped text: desktop icon labels (e.g. "Termina") are now centered and fully shown; taskbar clock
  no longer loses its last digit; Settings/USB/System windows anchor their controls below the taller body text.

## Build 4.9.13 — GUI terminal (M061)
- New desktop app "PSRE Terminal": Start menu entry, desktop icon, taskbar quick-launch, Ctrl+Alt+T and CLI `m061-open`.
- Runs every command of the recovery shell: `psre_m006_execute()` split out of the serial loop; its output is
  captured with the new `serial_set_sink()` tap and shown in the window (still mirrored to serial).
- Line editing (Left/Right/Home/End/Del/Backspace), history (Up/Down), scrollback (PgUp/PgDn, mouse wheel,
  256 lines), Ctrl+C / Ctrl+L, builtins `cls`, `exit`, `force CMD`.
- Commands that would break the running desktop (USB/input re-init, input polls, m050-pause) are blocked unless
  prefixed with `force`.
- Test: `make test-m061`.

## Build 4.9.12 — overlay painter (taskbar/widgets persist)
- M034 `psre_m034_set_overlay_painter()`: called on every render after windows, before the pointer.
- M035 draws application controls, then taskbar and Start menu through it. Previously input events re-rendered
  only the windows, so taskbar/widgets/app content disappeared after the first mouse move.
- Start menu now has 6 entries.

## Build 4.9.11 — smooth mouse
- M025: HID polling never blocks (timeout 0); events come from the xHCI event ring.
- M025: up to 16 queued mouse reports drained per poll; consecutive relative moves coalesced into one event
  (one desktop render per batch instead of per report).

## Build 4.9.10 — flicker-free desktop (double buffering)
- M031: off-screen back buffer (PMM contiguous pages via HHDM); `psre_m031_enable_backbuffer/present`.
  Drawing no longer touches the visible framebuffer; present copies only dirty rows once per main-loop pass.
- M031: `fill_rect` rewritten (direct 32-bit row fill) instead of per-pixel put_pixel (full-screen clear was slow
  and visibly flashed).
- main.c: enable back buffer at desktop handoff; present after `psre_m050_run_once()`.
- Test: `make test-m060`.

## Build 4.9.9 — multi-device USB HID
- M018/M020 re-enumerate per device (previously one attempt per xHCI init).
- M022 endpoints keyed by (slot, address); new `psre_m022_interrupt_in_slot`. Fixes keyboard/mouse both using 0x81.
- M025 `psre_m025_add_device()`; M049 scans all ports until keyboard and mouse are active.
- Slot Context "Context Entries" raised to the new endpoint DCI.
- `make run` adds qemu-xhci + usb-kbd + usb-mouse (`QEMU_USB=` to override).
- Version centralised in `kernel/include/psre/version.h`; banner prints version and tag.

## Build 4.9.8 — USB input fix
- `M049_AUTOSTART` default 0 -> 1 (USB input was never started).
- M022: interrupt TRB stays armed after idle timeout (endpoint no longer wedges), Short Packet (cc 13) accepted,
  transfer ring wraps via Link TRB, events routed per endpoint.
- M049 tries the next port if a device is not HID. Removed duplicate `psre_m058_initialize()` call.

## Build 4.9.7 — M058
- Added read-only xHCI/HID/input audit.
- Clarified M022 Endpoint Context CErr and Interrupt-IN field encoding.
- Preserved the existing Desktop boot, framebuffer handoff, and M050 event loop.
