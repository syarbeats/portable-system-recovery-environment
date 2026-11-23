# Build 4.9.3.1 — Limine Framebuffer Request ID Hotfix

## Root cause found in Build 4.9.3 source
- The framebuffer request ID was incorrectly defined as four framebuffer-specific words. The Limine protocol requires the two common magic words followed by the two framebuffer-specific words.
- The request-start delimiter contained only two words. The protocol start marker is four 64-bit words. With the requested base revision 4, delimiters must be honoured, so the incomplete marker can prevent requests from being discovered.
- The previous serial banner still identified the kernel as Build 4.9.2, making it difficult to tell which kernel was actually booted.

## Fixes
- Corrected `LIMINE_FRAMEBUFFER_REQUEST_ID` to the official four-word ID.
- Corrected the start marker to all four protocol words.
- Kept the correct two-word end marker and base revision tag.
- Updated kernel serial banner, Limine menu label, Makefile help, and README to Build 4.9.3.1.

## Validation limits
Source-level checks and host syntax checks do not prove a successful x86_64 cross-link or runtime response. A fresh `make clean && make iso` and OVMF boot are required. Do not reuse an older ISO.
