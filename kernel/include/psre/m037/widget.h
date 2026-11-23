#ifndef PSRE_M037_WIDGET_H
#define PSRE_M037_WIDGET_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m030/input.h>
#include <psre/m032/graphics.h>
#define PSRE_M037_MAX_WIDGETS 32u
#define PSRE_M037_TEXT_CAPACITY 96u
#define PSRE_M037_INVALID_WIDGET 0xFFu
typedef uint8_t psre_m037_widget_id_t;
typedef enum { PSRE_M037_LABEL=0, PSRE_M037_BUTTON, PSRE_M037_TEXTBOX, PSRE_M037_CHECKBOX } psre_m037_widget_type_t;
typedef void (*psre_m037_action_t)(psre_m037_widget_id_t widget, void *context);
typedef struct {
 bool initialized; uint8_t widget_count; psre_m037_widget_id_t focused_widget;
 uint32_t events_received,events_handled,clicks,key_inputs,redraws,rejected_events;
} psre_m037_status_t;
bool psre_m037_initialize(void); void psre_m037_reset(void);
psre_m037_widget_id_t psre_m037_add_label(const char*,int32_t,int32_t,uint32_t,psre_m032_color_t);
psre_m037_widget_id_t psre_m037_add_button(const char*,int32_t,int32_t,int32_t,int32_t,psre_m037_action_t,void*);
psre_m037_widget_id_t psre_m037_add_textbox(const char*,int32_t,int32_t,int32_t,int32_t,uint16_t);
psre_m037_widget_id_t psre_m037_add_checkbox(const char*,int32_t,int32_t,bool);
bool psre_m037_set_text(psre_m037_widget_id_t,const char*);
bool psre_m037_get_text(psre_m037_widget_id_t,char*,uint32_t);
bool psre_m037_set_checked(psre_m037_widget_id_t,bool);
bool psre_m037_is_checked(psre_m037_widget_id_t,bool*);
bool psre_m037_set_enabled(psre_m037_widget_id_t,bool);
bool psre_m037_focus(psre_m037_widget_id_t);
bool psre_m037_handle_event(const psre_input_event_t*);
bool psre_m037_render(void);
const psre_m037_status_t *psre_m037_get_status(void); void psre_m037_report(void);
#endif
