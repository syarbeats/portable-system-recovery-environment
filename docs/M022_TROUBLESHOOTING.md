# M022 Troubleshooting

- `Run m019-config and m021-probe first`: fetch configuration descriptors and parse HID report descriptors before selecting an endpoint.
- `No validated HID Interrupt IN endpoint found`: inspect M019 endpoint addresses, transfer type and direction; only Interrupt IN is accepted.
- Configure Endpoint timeout: verify M015 is RUNNING, M018 slot/device context is valid, and the controller accepts the endpoint context.
- Transfer timeout: check endpoint interval, device connection, packet size, controller event ring health and port status.
- A failed transfer does not automatically reset or dequeue the endpoint. Reinitialize the controller/device before retrying after uncertain hardware state.
- USB keyboard/mouse input is not yet integrated; `m022-read` only prints raw report bytes.
