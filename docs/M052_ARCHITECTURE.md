# M052 Architecture

`usb_hub.c` is a freestanding, transport-independent hub/topology module.

```text
Hub descriptor bytes -> validated descriptor model
Hub interrupt bitmap -> downstream port-change events
Hub registration -> bounded parent/child topology
Device registration -> multi-device registry
                         -> bounded event FIFO
```

The module deliberately does not issue USB requests or touch xHCI MMIO. A future hub transport driver must retrieve descriptors, power ports, schedule the hub interrupt-IN endpoint, clear change features, and run the xHCI slot/address/configuration sequence for each downstream device before registering it with M052.

Limits: 8 hubs, 15 ports/hub, 5 hub levels, 32 device records, 64 queued events.
