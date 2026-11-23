# M017 Architecture

M017 adds `kernel/src/m017/usb_enum.c`. It consumes M015's running controller and ring infrastructure rather than replacing DMA ring registers while the controller is active.

M015 exposes two serialized polling primitives:
- `psre_m015_submit_command`: writes one command TRB with the current producer cycle and rings doorbell 0.
- `psre_m015_wait_command_event`: checks the next event TRB cycle, returns one event, advances the dequeue pointer and updates ERDP.

M017 reads the selected PORTSC through a temporary UC MMIO mapping, requires CCS and PED, unmaps that mapping, then submits Enable Slot. It accepts only a Command Completion Event with successful completion code and non-zero Slot ID.

No Device Context or endpoint transfer is attempted in this milestone. M018 should implement Address Device and endpoint-0 control transfers, with context-size awareness and DMA lifetime management.
