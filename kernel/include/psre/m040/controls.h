#ifndef PSRE_M040_CONTROLS_H
#define PSRE_M040_CONTROLS_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m038/container.h>
#include <psre/m030/input.h>
#define PSRE_M040_MAX_CONTROLS 32u
#define PSRE_M040_TEXT_CAPACITY 64u
#define PSRE_M040_MAX_ITEMS 12u
#define PSRE_M040_INVALID_CONTROL 0xFFu
typedef uint8_t psre_m040_control_id_t;
typedef enum { PSRE_M040_COMBO=0, PSRE_M040_LIST, PSRE_M040_TABLE, PSRE_M040_SCROLLBAR, PSRE_M040_RADIO_GROUP, PSRE_M040_PROGRESS, PSRE_M040_SLIDER } psre_m040_type_t;
typedef struct { bool initialized, modal_visible; uint8_t control_count; uint32_t events, changes, redraws, errors; } psre_m040_status_t;
bool psre_m040_initialize(void);
void psre_m040_reset(void);
psre_m040_control_id_t psre_m040_create(psre_m038_container_id_t container, psre_m040_type_t type, const char *title, int32_t x, int32_t y, int32_t width, int32_t height);
bool psre_m040_add_item(psre_m040_control_id_t id, const char *text);
bool psre_m040_set_value(psre_m040_control_id_t id, int32_t value);
bool psre_m040_get_value(psre_m040_control_id_t id, int32_t *value);
bool psre_m040_set_range(psre_m040_control_id_t id, int32_t minimum, int32_t maximum);
bool psre_m040_set_table_cell(psre_m040_control_id_t id, uint8_t row, uint8_t column, const char *text);
bool psre_m040_clear_table(psre_m040_control_id_t id);
bool psre_m040_get_table_selection(psre_m040_control_id_t id, uint8_t *row);
bool psre_m040_set_modal(bool visible, const char *title, const char *message);
bool psre_m040_handle_event(const psre_input_event_t *event);
bool psre_m040_render(void);
bool psre_m040_render_container(psre_m038_container_id_t container);
const psre_m040_status_t *psre_m040_get_status(void);
void psre_m040_report(void);
#endif
