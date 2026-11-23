# M017 Troubleshooting

- **M015 not RUNNING:** execute M014 probe, then explicit `m015-init`.
- **Port not enabled:** run `m016-ports`; only a connected port with PED set is accepted. Run `m016-reset PORT` first.
- **Command submission rejected:** verify M015 state and avoid concurrent xHCI command producers.
- **No Command Completion Event:** check controller state, command ring/doorbell offsets and serial timeout stage. A timeout does not prove the command was cancelled; avoid blind repeated submissions.
- **Slot allocation succeeds but no VID/PID appears:** expected in M017. Address Device and descriptor control transfers are future work.
