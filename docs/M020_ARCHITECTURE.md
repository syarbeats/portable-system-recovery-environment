# M020 Architecture

M020 uses M018's synchronous, polled EP0 control-transfer path to send no-data standard requests. `SET_CONFIGURATION` activates the configuration value previously parsed by M019. `SET_INTERFACE` selects a descriptor-listed interface alternate setting.

The M020 registry is statically allocated and bounded to four entries. The current upstream enumeration pipeline supports one active device, so only registry entry zero is populated. Interface classes are classified from Interface Descriptors, which is necessary for composite devices whose Device Descriptor class fields may be zero.

No class-specific requests or non-control endpoint transfers are issued. DMA pages remain pinned while the xHCI controller may still reference them.
