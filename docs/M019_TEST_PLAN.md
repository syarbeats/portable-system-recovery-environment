# M019 Test Plan

## Preconditions
1. Boot PSRE with serial console.
2. Run `m014-probe`, `m015-init`.
3. Run `m016-ports` and reset a connected supported port with `m016-reset PORT`.
4. Run `m017-enable-slot PORT`.
5. Run `m018-enumerate PORT` and confirm Device Descriptor success.

## M019 test
1. Run `m019-config`.
2. Run `m019-info`.
3. Confirm configuration total length, configuration value, interfaces, endpoints, and any strings.
4. Repeat with a second USB device only after controller reinitialization; this build supports one addressed device/one EP0 ring lifecycle.

## Negative tests
- Run before successful M018: M019 must reject.
- Invalid descriptor header/length: parser must reject.
- Descriptor length over 1024: reject before copy.
- Malformed descriptor with bLength < 2 or beyond remaining bytes: reject.
- More than 16 interfaces or 32 endpoints: reject.

## Validation caveat
Host compilation and static checks do not verify xHCI transfer behavior. QEMU and physical-device runtime tests are still required.
