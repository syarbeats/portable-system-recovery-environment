# M030 Architecture

```text
USB HID device
   -> xHCI M022 transfer engine
   -> M023/M027 keyboard and M028 mouse decoders
   -> M025 unified HID event FIFO
   -> M030 router
        |-> 128-entry M030 FIFO -> application dequeue API
        |-> up to 8 synchronous subscribers
        |-> keyboard/mouse state snapshot
```

M030 owns no USB DMA memory and does not access PCI or xHCI registers. A call
to `psre_m030_poll(timeout)` polls M025 once and drains all queued M025 events.
Each event updates state, is enqueued if capacity remains, and is dispatched to
matching callbacks. FIFO overflow does not suppress callback delivery.

The event callback is invoked synchronously from `psre_m030_poll()`. Callbacks
must be short and non-blocking. The current kernel has no SMP locks; callers
must serialize initialize, poll, subscribe, unsubscribe, dequeue and state
queries on the boot CPU.
