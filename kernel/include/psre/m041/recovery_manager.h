#ifndef PSRE_M041_RECOVERY_MANAGER_H
#define PSRE_M041_RECOVERY_MANAGER_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m038/container.h>
#define PSRE_M041_PATH_CAPACITY 128u
typedef enum {
    PSRE_M041_ACTION_NONE=0, PSRE_M041_ACTION_MOUNT, PSRE_M041_ACTION_LIST,
    PSRE_M041_ACTION_STAT, PSRE_M041_ACTION_CRC32, PSRE_M041_ACTION_RECOVER,
    PSRE_M041_ACTION_QUEUE_ADD, PSRE_M041_ACTION_QUEUE_LIST,
    PSRE_M041_ACTION_QUEUE_EXPORT, PSRE_M041_ACTION_QUEUE_CLEAR,
    PSRE_M041_ACTION_MANIFEST, PSRE_M041_ACTION_WORKSPACE, PSRE_M041_ACTION_EXPORT
} psre_m041_action_t;
typedef struct {
    bool initialized, mounted;
    psre_m038_container_id_t container;
    psre_m038_widget_id_t path_widget, status_widget;
    uint32_t actions, successes, failures;
    psre_m041_action_t last_action;
    char current_path[PSRE_M041_PATH_CAPACITY];
} psre_m041_status_t;
bool psre_m041_initialize(psre_m038_container_id_t container);
void psre_m041_reset(void);
bool psre_m041_set_path(const char *path);
bool psre_m041_execute(psre_m041_action_t action);
const psre_m041_status_t *psre_m041_get_status(void);
void psre_m041_report(void);
#endif
