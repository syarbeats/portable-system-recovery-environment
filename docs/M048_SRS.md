# M048 — GUI Boot Integration & Physical Hardware Support

## Objective
Introduce a boot coordinator that validates the framebuffer, optionally initializes xHCI, and starts the existing M034/M035 window manager and desktop only when the M030 HID input router is already initialized. Preserve serial recovery as the fallback.

## Boot behavior
1. M031 framebuffer must be available.
2. xHCI takeover is compile-time opt-in (`PSRE_M048_ENABLE_XHCI_AUTOSTART=1`); default is disabled because controller ownership/reset can disrupt firmware-managed USB devices.
3. When enabled, M015 initializes xHCI and M016 scans ports. This milestone does not claim automatic USB HID enumeration.
4. M030 must be initialized by a successfully enumerated/configured HID endpoint before M034/M035 are started.
5. Any failure is non-fatal and leaves the serial recovery path available.

## Current limitation
M017 only allocates an xHCI slot; it explicitly does not complete Address Device. Therefore M048 cannot truthfully claim plug-and-play keyboard/mouse support at boot. M018-M025 remain manually staged and platform-dependent. Full automatic enumeration, descriptor/configuration selection, boot-protocol HID setup, and an event-polling GUI loop are follow-on work.
