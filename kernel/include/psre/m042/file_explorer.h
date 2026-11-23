#ifndef PSRE_M042_FILE_EXPLORER_H
#define PSRE_M042_FILE_EXPLORER_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m038/container.h>
#include <psre/m040/controls.h>
#define PSRE_M042_PAGE_SIZE 6u
typedef enum { PSRE_M042_FILES=0, PSRE_M042_QUEUE=1 } psre_m042_view_t;
typedef struct {
    bool initialized, mounted, selected_valid, selected_checked;
    psre_m042_view_t view;
    psre_m038_container_id_t container;
    uint8_t page, selected_index;
    uint32_t directory_count, directory_total, queue_count, queue_bytes;
    uint32_t refreshes, stage_operations, queue_operations, errors;
    psre_m040_control_id_t table, progress;
} psre_m042_status_t;
bool psre_m042_initialize(psre_m038_container_id_t container);
bool psre_m042_refresh(void);
bool psre_m042_set_path(const char *path);
bool psre_m042_set_view(psre_m042_view_t view);
bool psre_m042_open_selected(void);
bool psre_m042_stage_selected(void);
bool psre_m042_queue_selected(void);
bool psre_m042_handle_event(const psre_input_event_t *event);
const psre_m042_status_t *psre_m042_get_status(void);
void psre_m042_report(void);
#endif
