# PSRE Integrated Build 4.9.4 — M001-M055

Cumulative base: Build 4.9.3.2 (M001-M054) plus M055 USB HID Input Polling & Desktop Integration.

Desktop-first invariant: M031 framebuffer and M048/M035 desktop startup are independent of USB controller and HID availability. M055 runs as a bounded M050 task and idles safely when no HID source is ready.

Known limitation: only the first enumerated USB device is currently retained by M049/M018/M019/M021 singleton state. Independent keyboard + mouse devices are not yet supported simultaneously.
