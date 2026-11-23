# M052 Test Plan

Host tests cover valid/invalid hub descriptor parsing, root and downstream hub registration, invalid parent rejection, hub interrupt bitmap decoding, multi-device registration, duplicate ID rejection, device removal, event FIFO consumption, and status counters.

Hardware tests still required: connect a USB 2.0 hub, obtain its descriptor through EP0, power a downstream port, observe hub interrupt-IN bitmap, clear port change features, enumerate two devices through separate xHCI slots, then unplug/reconnect each device. These are not claimed as passed by host tests.
