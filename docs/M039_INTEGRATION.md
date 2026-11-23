# M039 Integration

Include `<psre/m039/layout.h>`. M035 initializes M039 automatically.

```c
psre_m039_apply_vertical(container, 8, 10, true);
psre_m039_grid_config_t grid = {
    .columns = 2, .horizontal_gap = 8, .vertical_gap = 10,
    .padding = 12, .stretch_width = true
};
psre_m039_apply_grid(container, &grid);
psre_m039_redraw(container);
psre_m039_bring_widget_to_front(container, widget_id);
```

A container's last successful layout is reapplied by `psre_m039_reflow_all()`, called by M035 before rendering. Custom themes are passed to `psre_m039_set_theme()` after M039 initialization.
