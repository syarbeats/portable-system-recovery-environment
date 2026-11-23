# M050 Architecture

`PIT IRQ (M003) -> tick counter -> psre_m050_run_once()`

The serial CLI performs a nonblocking UART read. If no byte is available, it runs one scheduler pass and executes `hlt`; the PIT interrupt wakes the CPU. Registered tasks run when their deadlines expire. M049 input/GUI polling is a one-tick task, M043 process progress is a ten-tick task, and the device-state snapshot is refreshed every 100 ticks.

The scheduler is cooperative: a task that blocks delays all other tasks. No task may wait indefinitely for hardware. The M049 polling path remains subject to its existing USB transfer timeout behavior.
