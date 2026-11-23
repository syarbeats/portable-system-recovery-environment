#ifndef PSRE_M038_CONTAINER_H
#define PSRE_M038_CONTAINER_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m036/app.h>
#include <psre/m030/input.h>
#include <psre/m032/graphics.h>
#define PSRE_M038_MAX_CONTAINERS PSRE_M034_MAX_WINDOWS
#define PSRE_M038_MAX_WIDGETS 16u
#define PSRE_M038_TEXT_CAPACITY 96u
#define PSRE_M038_INVALID_CONTAINER 0xFFu
#define PSRE_M038_INVALID_WIDGET 0xFFu
typedef uint8_t psre_m038_container_id_t;
typedef uint8_t psre_m038_widget_id_t;
typedef enum {PSRE_M038_LABEL=0,PSRE_M038_BUTTON,PSRE_M038_TEXTBOX,PSRE_M038_CHECKBOX} psre_m038_widget_type_t;
typedef void (*psre_m038_action_t)(psre_m038_container_id_t container,
                                    psre_m038_widget_id_t widget,void *context);
typedef struct { psre_m032_color_t text,disabled,button,button_hover,button_pressed,button_border,accent,field_background,field_text,check; } psre_m038_theme_t;
typedef struct {
 bool used; psre_m036_app_id_t app; psre_m034_window_id_t window;
 uint8_t widget_count; psre_m038_widget_id_t focused_widget;
 uint32_t events,clicks,key_inputs,redraws;
} psre_m038_container_info_t;
typedef struct {
 bool initialized; uint8_t container_count;
 uint32_t events_received,events_handled,events_ignored,render_calls,render_errors;
} psre_m038_status_t;
bool psre_m038_initialize(void);
bool psre_m038_set_theme(const psre_m038_theme_t *theme);
void psre_m038_reset(void);
psre_m038_container_id_t psre_m038_create_container(psre_m036_app_id_t app);
bool psre_m038_destroy_container(psre_m038_container_id_t container);
bool psre_m038_reconcile(void);
bool psre_m038_get_container(psre_m038_container_id_t container,psre_m038_container_info_t *out);
psre_m038_widget_id_t psre_m038_add_label(psre_m038_container_id_t container,const char *text,
                                           int32_t x,int32_t y,uint32_t width);
psre_m038_widget_id_t psre_m038_add_button(psre_m038_container_id_t container,const char *text,
                                            int32_t x,int32_t y,int32_t width,int32_t height,
                                            psre_m038_action_t action,void *context);
psre_m038_widget_id_t psre_m038_add_textbox(psre_m038_container_id_t container,const char *text,
                                             int32_t x,int32_t y,int32_t width,int32_t height,
                                             uint16_t max_length);
psre_m038_widget_id_t psre_m038_add_checkbox(psre_m038_container_id_t container,const char *text,
                                              int32_t x,int32_t y,uint32_t width,bool checked);
bool psre_m038_set_text(psre_m038_container_id_t container,psre_m038_widget_id_t widget,const char *text);
bool psre_m038_get_text(psre_m038_container_id_t container,psre_m038_widget_id_t widget,char *out,uint32_t capacity);
bool psre_m038_set_checked(psre_m038_container_id_t container,psre_m038_widget_id_t widget,bool checked);
bool psre_m038_is_checked(psre_m038_container_id_t container,psre_m038_widget_id_t widget,bool *checked);
bool psre_m038_set_enabled(psre_m038_container_id_t container,psre_m038_widget_id_t widget,bool enabled);
bool psre_m038_get_widget_geometry(psre_m038_container_id_t container,psre_m038_widget_id_t widget,int32_t *x,int32_t *y,int32_t *width,int32_t *height);
bool psre_m038_set_geometry(psre_m038_container_id_t container,psre_m038_widget_id_t widget,int32_t x,int32_t y,int32_t width,int32_t height);
bool psre_m038_bring_to_front(psre_m038_container_id_t container,psre_m038_widget_id_t widget);
bool psre_m038_render_container(psre_m038_container_id_t container);
bool psre_m038_focus(psre_m038_container_id_t container,psre_m038_widget_id_t widget);
bool psre_m038_handle_event(const psre_input_event_t *event);
bool psre_m038_render(void);
const psre_m038_status_t *psre_m038_get_status(void);
void psre_m038_report(void);
#endif
