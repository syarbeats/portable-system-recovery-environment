# M061 GUI Terminal

Open it from **Start > Terminal**, the **Terminal** desktop icon, the **>** taskbar button, or **Ctrl+Alt+T**
(serial CLI: `m061-open`).

The prompt runs the same command set as the serial recovery shell (`help` lists everything: `ls`, `cat`,
`mount`, `usb-info`, `m015-info`, `m058-*`, ...). Output appears in the window and is also mirrored to COM1.

| Keys | Action |
|---|---|
| Enter | run the line |
| Up / Down | history (16 entries) |
| Left / Right / Home / End / Del / Backspace | edit |
| PgUp / PgDn, mouse wheel | scrollback (256 lines) |
| Scroll bar (right edge) | arrow buttons = 1 line, drag the thumb, click the track = 1 page |
| Ctrl+C / Ctrl+L | cancel line / clear window |

Terminal builtins: `cls` (clear window; the shell's `clear` frees the RAM workspace), `exit`, `force CMD`.

Some commands would break the running desktop because they re-initialize the USB/input stack the terminal
itself depends on (`m015-init`, `m016-reset`, `m018-enumerate`, `m025-start`, `m030-start`, `xhci-release`,
`m050-pause`, input poll commands ...). They are blocked with an explanation; `force CMD` runs them anyway.
`halt` stops the CPU as in the serial shell.

## Text size

The desktop (and terminal) use a larger font by default. Switch with `m033-font normal` or `m033-font large`
(`m033-font` alone shows the current size). It works from the GUI terminal and from the serial shell.
