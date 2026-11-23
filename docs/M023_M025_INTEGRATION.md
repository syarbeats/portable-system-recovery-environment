# PSRE M023–M025 Integration

M023 adds HID boot keyboard report decoding. M024 adds HID boot mouse report decoding. M025 chooses one supported interface, configures it through M022, polls reports and queues common input events.

Recommended sequence:
1. `m015-init`
2. `m016-ports`
3. `m016-reset PORT`
4. `m017-enable-slot PORT`
5. `m018-enumerate PORT`
6. `m019-config`
7. `m020-configure`
8. `m021-probe`
9. `m025-start`
10. Repeat `m025-poll` and `m025-next`; use `m025-info`, `m023-info`, and `m024-info` for diagnostics.

Do not run `m022-configure` first: M025 configures the selected endpoint itself. M022 currently supports only one endpoint for the device/controller lifecycle. Therefore the input manager selects keyboard preferentially, otherwise mouse; it does not poll a keyboard and mouse simultaneously.

Keyboard reports are boot-protocol 8-byte reports; mouse reports are boot-protocol 3-byte reports. The current CLI requires explicit polling. This milestone does not yet provide background input, ASCII key translation, GUI cursor, wheel support, or multiple HID endpoint scheduling.
