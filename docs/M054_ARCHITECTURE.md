# M054 Architecture

M015 remains the owner of controller command/event rings, DMA allocation and DCBAA. M054 adds a bounded registry indexed by Slot ID. Each registered slot receives independent device-context and EP0-ring DMA pages and its DCBAA entry is updated. Existing M017/M018 remain single-active-device implementations; M054 does not silently reuse their static transfer state for another device.

Boot order for GUI-first attempt:
1. M031 framebuffer discovery.
2. Memory and interrupt initialization.
3. M014 xHCI discovery.
4. M049 USB HID auto-enumeration attempt (default build setting enabled).
5. M048 GUI readiness integration.
6. M050 cooperative event loop.
7. M054 slot registry initialization.
8. M006 serial recovery CLI, whose idle wait continues to run M050.

If framebuffer, xHCI, enumeration, or HID setup fails, boot continues to serial recovery. The default autostart may take ownership of xHCI from firmware; use `make M049_AUTOSTART=0` to disable that behavior.
