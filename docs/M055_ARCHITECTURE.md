# M055 Architecture

M050 PIT-driven task -> M055 bounded poll service -> M049 event pump -> M030 router -> M034 window manager subscription and M035 shell event handling.

M025 polls configured interrupt-IN endpoints through M022. Keyboard reports are parsed by M023/M027. Mouse reports use M028 when a report-descriptor interface is available, otherwise M024 boot protocol.

M055 is deliberately non-fatal: no controller, no HID device, failed transfer, or unavailable mouse does not stop M031/M035 desktop boot.
