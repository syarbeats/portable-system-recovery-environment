# Build 4.9.13 — GUI terminal (M061)
- psre_m006_execute(line): command dispatcher extracted from the serial loop (same command set).
- serial_set_sink(): tap on serial output used to capture command output into the terminal.
- M061 terminal window: history, editing, scrollback, builtins (cls, exit, force).
- Protected commands: m015-init, xhci-release, m016-reset, m017-enable-slot, m018-enumerate, m020-configure,
  m020-alt, m025/m026/m030-start, m030/m034/m035/m049-poll, m050-pause (use `force CMD` to override).
- Launch: Start menu > Terminal, desktop icon, taskbar button, Ctrl+Alt+T, serial `m061-open`.
- Test: tests/m061.
