# PSRE Integrated Build 1.5 — M001 to M018

Build 1.5 preserves the cumulative M001–M017 source tree and adds M018 USB Address Device and Device Descriptor reading. The single root Makefile includes `kernel/src/m018/usb_enumerate.c`.

Recommended explicit sequence:
1. `m014-probe`
2. `m015-init`
3. `m016-ports`
4. `m016-reset PORT`
5. `m017-enable-slot PORT`
6. `m018-enumerate PORT`
7. `m018-info`

M018 currently identifies one USB device by reading its Device Descriptor. Configuration descriptors, strings, class drivers and USB Mass Storage are future work. Hardware runtime validation is not claimed by static compilation.
