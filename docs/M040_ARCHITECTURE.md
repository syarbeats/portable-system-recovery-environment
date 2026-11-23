# M040 Architecture

`psre/m040/controls.h` exposes a bounded control registry. Each control stores its M038 container ID, type, local geometry, title, value/range, and fixed-size item/cell buffers. `controls.c` resolves the current M034 window geometry at render/input time, so controls follow window movement without storing screen coordinates.

Rendering uses M032 primitives and M033 text. Input uses M030 pointer state and the existing M035 callback chain. The modal dialog is a global overlay. The control layer is additive: M038 remains responsible for the base window widgets and M039 remains responsible for layout/reflow.
