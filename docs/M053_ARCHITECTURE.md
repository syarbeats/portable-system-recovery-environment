# M053 Architecture

M053 is layered over M018 control transfers and M052 topology registry. The active USB device must already be configured and classified as a Hub by M020. M053 fetches descriptor type 0x29, registers the Hub in M052, requests port power, and exposes explicit port status scan/reset/change-clear operations.

The M018 EP0 context is global/single-device in this build. Therefore M053 is a single active-hub class driver foundation, not recursive hub enumeration.
