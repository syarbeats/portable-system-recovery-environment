# Build 4.9.12 — overlay painter
Bug: input events re-rendered only M034 windows; taskbar, Start menu and M038/M040 widget content were drawn by
psre_m035_render() only (launch time), so they vanished after the next mouse move.
Fix: M034 overlay painter hook (after windows, before pointer); M035 registers it and draws widgets, then taskbar/menu.
Test: tests/m034 (overlay assertions), tests/m035.
