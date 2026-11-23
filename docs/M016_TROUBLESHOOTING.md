# M016 Troubleshooting

- **M015 is not RUNNING**: run `m015-info`; M016 refuses to access PORTSC unless M015 reached RUNNING with Bus Master enabled.
- **No port inventory**: inspect M015 MaxPorts and BAR mapping. M016 maps only the required port register window.
- **Disconnected port**: `m016-reset` is rejected unless PORTSC.CCS is set.
- **Over-current**: reset is rejected when PORTSC.OCA is set. Inspect the port/device/power source.
- **Power remains off**: HCCPARAMS1.PPC may not be advertised, or the platform may manage port power elsewhere.
- **Unknown speed ID**: reset is rejected; inspect controller model and port protocol.
- **PR/WPR timeout**: inspect PORTSC, xHCI USBSTS, device connection and controller state. A reset timeout is not proof that the device is defective.
- **PED not set after reset**: reset completion alone does not guarantee a device can be enumerated. Descriptor/slot operations are later milestones.
