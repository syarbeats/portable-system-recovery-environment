# Build 4.9.3 — Limine Request Registration & Framebuffer Response Recovery

## Changes
- Consolidated Limine request start marker, all request objects, and end marker into one contiguous `.limine_requests` output section.
- Placed that section in the writable `data` PT_LOAD segment so the bootloader can populate response pointers.
- Added explicit 8-byte alignment and `used` attributes to request objects and markers.
- Added compile-time ABI assertions for the Limine framebuffer request structure (48-byte size; response pointer at byte offset 40).
- Kept the existing Limine framebuffer request ID and revision 0; did not invent a new protocol ID.
- Updated Limine menu label and requested mode to `1024x768x32`.
- Added post-link ELF validation for request section, writable flag, marker boundaries, and request symbols.
- Added host source-registration checks.

## Validation limits
Host source checks do not prove that the installed Limine binary release recognizes the request or that OVMF supplies a framebuffer response. A real cross-link, ELF inspection, ISO build, QEMU/OVMF boot, and physical PC test remain required.
