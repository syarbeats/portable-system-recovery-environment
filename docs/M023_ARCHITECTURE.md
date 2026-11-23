# M023 Architecture

M023 depends on M021's HID interface classification and M018's EP0 control request path. It selects a HID boot keyboard interface (Subclass 1 / Protocol 1), issues `SET_PROTOCOL(Boot)`, then decodes each 8-byte boot report into modifier bits and six keycode slots. M025 performs state comparison to generate key-down/key-up events.

The module does not own the xHCI transfer ring; M022 owns endpoint configuration and Interrupt IN transfers.
