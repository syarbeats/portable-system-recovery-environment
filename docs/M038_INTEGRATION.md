# M038 Integration

- Public API: `kernel/include/psre/m038/container.h`
- Implementation: `kernel/src/m038/container.c`
- Host tests: `make test-m038`
- Dependencies: M034 Window Manager, M036 Application Framework, M032 graphics, M033 fonts, M030 input event definitions.
- M035 initializes M038, creates a container for each built-in app at launch, routes events to the focused window container, renders containers after M034 and reconciles stopped apps.
- M037 remains as the legacy global widget layer; M038 is the new per-window layer.
