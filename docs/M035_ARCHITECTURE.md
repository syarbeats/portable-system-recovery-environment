# M035 — Architecture

M035 registers `psre_m035_draw_desktop` as the M034 desktop painter. M034 invokes this painter after clearing the framebuffer and before drawing z-ordered windows. M034 also draws the software mouse pointer. M035 subscribes to M030 input events after M034, so the event path is M030 -> M034 -> M035.

M035 draws taskbar and Start Menu chrome after M034's render. Desktop icons are painted by the background painter, therefore they remain behind windows. App launchers use the M034 window API and a fixed table of four window IDs. The clock converts the 100 Hz PIT tick count to HH:MM:SS uptime.
