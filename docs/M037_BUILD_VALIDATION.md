# M037 — Build Validation
Host tests stub M030, M032 and M033. Tests cover registration, rendering, focus, keyboard activation, mouse hit-testing, checkbox state, textbox editing and enable/disable. Host validation does not prove framebuffer runtime, USB HID operation, cross-link, QEMU boot or physical hardware behavior. Those require the x86_64-elf toolchain, NASM, QEMU and target hardware.
