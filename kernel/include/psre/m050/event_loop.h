#ifndef PSRE_M050_EVENT_LOOP_H
#define PSRE_M050_EVENT_LOOP_H
#include <stdbool.h>
#include <stdint.h>
#define PSRE_M050_MAX_TASKS 8u
typedef void (*psre_m050_task_fn)(void *context);
typedef uint8_t psre_m050_task_id_t;
typedef struct {
    bool initialized, running;
    uint8_t task_count;
    uint64_t loop_iterations, task_runs, missed_deadlines;
    uint64_t last_tick;
} psre_m050_status_t;
bool psre_m050_initialize(void);
bool psre_m050_register(psre_m050_task_fn fn, void *context, uint32_t period_ticks,
                        psre_m050_task_id_t *id);
bool psre_m050_unregister(psre_m050_task_id_t id);
void psre_m050_run_once(void);
void psre_m050_set_running(bool running);
const psre_m050_status_t *psre_m050_get_status(void);
void psre_m050_report(void);
#endif
