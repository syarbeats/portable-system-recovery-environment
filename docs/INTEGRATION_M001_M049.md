# PSRE Integrated Build 4.4 — M001–M049

Cumulative source tree based on Build 4.3 (M001–M048), with M049 added. M049 adds guarded one-device USB HID auto-enumeration, M030 input initialization, GUI startup, and an explicit GUI event pump. Existing milestones and their source files are retained.

Important: automatic USB takeover is disabled by default. Build with `M049_AUTOSTART=1` only for controlled hardware testing. Serial recovery CLI remains the fallback. See M049 SRS, Architecture, Test Plan, Integration, and Build Validation documents.
