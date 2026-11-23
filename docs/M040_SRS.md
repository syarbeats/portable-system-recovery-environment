# M040 — GUI Application Controls & Advanced Interaction

## Objective
Extend the M038 window-scoped widget containers with higher-level GUI controls for recovery and system-management applications.

## Functional requirements
- ComboBox: bounded item list, current selection, open/close state.
- ListView: bounded selectable item list.
- TableView: up to 12 rows and three text columns.
- Scrollbar: integer range/value model.
- Radio Group: bounded mutually-exclusive item selection.
- Progress Bar: bounded integer range/value display.
- Slider: bounded integer range/value with mouse interaction.
- Modal dialog: title/message overlay, dismissed by Enter, Escape, or mouse click.
- Controls are associated with an existing M038 container and use client-area-relative coordinates.

## Constraints
- Static allocation only: 32 controls, 12 list/table rows, 64 bytes per text field.
- No heap allocation, threads, accessibility tree, clipboard, Unicode shaping, or nested controls.
- Controls are kernel-resident GUI components, not isolated user processes.
- Input is routed through the existing M030 subscription path.
