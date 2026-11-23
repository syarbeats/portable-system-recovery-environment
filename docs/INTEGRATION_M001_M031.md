# PSRE Integration M001–M031

M031 is added to the cumulative source tree and root Makefile. `limine_requests.c` registers the framebuffer request. `main.c` initializes M031 after memory-map/HHDM acquisition; failure is non-fatal. The serial recovery shell provides `m031-info` and explicit `m031-pattern` commands.
