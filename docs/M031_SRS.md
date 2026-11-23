# M031 — Framebuffer Manager (SRS)

Acquire the framebuffer supplied by Limine, validate its geometry and RGB pixel layout, and expose bounded pixel/rectangle drawing APIs.

Requirements: request the Limine framebuffer; use the first framebuffer; accept RGB model at 24/32 bpp; validate dimensions, pitch, overflow, total size (maximum 1 GiB), and channel masks; expose metadata; provide bounded pixel and rectangle writes; never modify framebuffer memory automatically during boot; retain serial recovery if no valid framebuffer exists.

Non-goals: font rendering, terminal, double buffering, compositor, page flipping, accelerated graphics, GOP driver, and GUI widgets.
