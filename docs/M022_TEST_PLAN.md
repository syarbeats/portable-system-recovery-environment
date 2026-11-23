# M022 Test Plan

## Static/build tests
1. Compile every C translation unit with `-std=gnu11 -ffreestanding -Wall -Wextra -Werror`.
2. Compile C++ subsystem with project flags.
3. Run `make -n all`.
4. Verify M022 source/header/CLI/Makefile integration and ZIP integrity.

## Hardware/QEMU tests (not performed by host compile)
1. Boot with serial console and a supported xHCI controller.
2. Run `m015-init`, `m016-ports`, reset the connected port, then run M017/M018 enumeration, M019 configuration and M021 HID probe.
3. Run `m022-configure`; expect Configure Endpoint completion code Success.
4. Run `m022-read`; verify Transfer Event success, residual length and report bytes.
5. Repeat reads to exercise ring enqueue and Link TRB wrap.

## Negative tests
- OUT endpoint, endpoint zero, endpoint number zero, zero interval, zero packet size, oversized packet size.
- Missing M018 address assignment or non-running M015 controller.
- No HID Interrupt IN endpoint in descriptors.
- Disconnect/timeout while transfer is pending.

Physical tests are required before declaring hardware support.
