# PSRE M001–M030 Integration Notes

Build 2.5 is cumulative from Build 2.4 (M001–M029). M030 adds the independent
kernel input event router and state API while retaining all prior milestones.
The root Makefile remains the single build entry point.

## Bring-up sequence
1. `m015-init`
2. `m016-ports`
3. `m016-reset PORT`
4. `m017-enable-slot PORT`
5. `m018-enumerate PORT`
6. `m019-config`
7. `m020-configure`
8. `m021-probe`
9. `m030-start`
10. Repeat `m030-poll`; read events with `m030-next`; inspect with `m030-info`.

The USB setup sequence depends on the existing one-device/one-lifecycle limits
of earlier milestones. M030 does not remove those constraints.
