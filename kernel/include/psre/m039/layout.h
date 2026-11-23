#ifndef PSRE_M039_LAYOUT_H
#define PSRE_M039_LAYOUT_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m038/container.h>
#include <psre/m032/graphics.h>
#define PSRE_M039_MAX_RULES PSRE_M038_MAX_WIDGETS
typedef enum { PSRE_M039_VERTICAL=0, PSRE_M039_HORIZONTAL=1, PSRE_M039_GRID=2 } psre_m039_layout_mode_t;
typedef struct { uint8_t mode, columns; uint16_t gap, gap_y, padding; bool stretch; } psre_m039_layout_config_t;
typedef struct { uint32_t layouts_applied, widgets_positioned, layout_errors, redraws; } psre_m039_status_t;
typedef struct { psre_m032_color_t text,disabled,button,button_hover,button_pressed,button_border,accent,field_background,field_text,check; } psre_m039_theme_t;
typedef struct { uint8_t columns; uint16_t horizontal_gap, vertical_gap, padding; bool stretch_width; } psre_m039_grid_config_t;
bool psre_m039_initialize(void);
bool psre_m039_set_theme(const psre_m039_theme_t *theme);
bool psre_m039_apply_layout(psre_m038_container_id_t container,const psre_m039_layout_config_t *config);
bool psre_m039_apply_vertical(psre_m038_container_id_t container,uint16_t gap,uint16_t padding,bool stretch_width);
bool psre_m039_apply_horizontal(psre_m038_container_id_t container,uint16_t gap,uint16_t padding);
bool psre_m039_apply_grid(psre_m038_container_id_t container,const psre_m039_grid_config_t *config);
bool psre_m039_reflow_all(void);
bool psre_m039_redraw(psre_m038_container_id_t container);
bool psre_m039_bring_widget_to_front(psre_m038_container_id_t container,psre_m038_widget_id_t widget);
const psre_m039_status_t *psre_m039_get_status(void);
void psre_m039_report(void);
#endif
