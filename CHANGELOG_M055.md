# CHANGELOG M055

- Added M055 non-fatal HID polling service.
- Connected M055 to M050 PIT-driven task scheduling.
- Added boot-protocol mouse M024 fallback in M025 when M028 report-descriptor parsing is unavailable.
- Removed duplicate M034 event dispatch in M049; M034 already subscribes to M030 synchronously.
- Kept GUI startup independent of USB/HID readiness.
- Documented remaining one-USB-device enumeration limitation.
