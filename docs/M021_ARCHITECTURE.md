# M021 Architecture

M019 now retains HID class descriptor metadata for each HID interface, including the Report Descriptor length. M021 selects alternate setting zero, finds an Interrupt IN endpoint among the interface's endpoint descriptors, then requests the Report Descriptor through M018's EP0 control-IN helper.

The parser supports HID short items and skips bounded long items. It tracks Global Usage Page, Report Size, Report ID and Report Count; Local Usage; and Main Input items. Input bit counts are accumulated per Report ID. Keyboard and mouse identification is inferred from the relevant HID usage page/usage items.

This is not yet an operational HID data path. xHCI Configure Endpoint and interrupt transfer rings require endpoint context updates, command completion handling, transfer event routing, and a safe controller-owned DMA lifecycle; those are deferred to the next milestone.
