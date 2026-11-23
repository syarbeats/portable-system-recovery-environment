# M028 Architecture

M028 is a class-level HID mouse parser above M022. It retrieves the active interface's Report Descriptor through M018 EP0 control transfers, builds a bounded table of relevant Input fields and per-report bit offsets, accepts relative motion fields, and decodes each interrupt-IN report delivered by M022.

M025 now initializes M028 alongside the M023/M027 keyboard path. The mouse endpoint is configured independently through `psre_m022_configure_interrupt_in_multi`. Each successful poll is passed to M028; resulting X/Y motion, button transitions (1–8), vertical wheel and horizontal pan are emitted into the existing 64-entry FIFO.

M028 chooses Report Protocol for subclass-1 interfaces using SET_PROTOCOL(1). Non-boot HID mouse interfaces are assumed to use Report Protocol already. A dedicated mouse interface is required; combined keyboard+mouse interfaces are excluded to avoid changing the protocol expected by the Boot Keyboard driver.
