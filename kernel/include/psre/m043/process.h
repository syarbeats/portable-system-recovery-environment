#ifndef PSRE_M043_PROCESS_H
#define PSRE_M043_PROCESS_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m038/container.h>
#define PSRE_M043_HISTORY_CAPACITY 16u
#define PSRE_M043_PATH_CAPACITY 128u
typedef enum { PSRE_M043_IDLE=0, PSRE_M043_RUNNING, PSRE_M043_PAUSED, PSRE_M043_COMPLETED, PSRE_M043_FAILED } psre_m043_state_t;
typedef enum { PSRE_M043_JOB_NONE=0, PSRE_M043_JOB_STAGE_FILE, PSRE_M043_JOB_EXPORT_QUEUE } psre_m043_job_t;
typedef struct { uint32_t id; psre_m043_job_t job; psre_m043_state_t result; bool success; char path[PSRE_M043_PATH_CAPACITY]; uint32_t bytes_done, bytes_total, attempts; } psre_m043_history_t;
typedef struct { bool initialized; psre_m043_state_t state; psre_m043_job_t job; uint32_t job_id, files_done, files_total, bytes_done, bytes_total, steps, retries, errors; char path[PSRE_M043_PATH_CAPACITY]; psre_m043_history_t history[PSRE_M043_HISTORY_CAPACITY]; uint8_t history_count, history_next; } psre_m043_status_t;
bool psre_m043_initialize(psre_m038_container_id_t container);
bool psre_m043_start_stage(const char *path);
bool psre_m043_start_queue_export(void);
bool psre_m043_pause(void);
bool psre_m043_resume(void);
bool psre_m043_retry(void);
void psre_m043_tick(void);
void psre_m043_cancel(void);
const psre_m043_status_t *psre_m043_get_status(void);
void psre_m043_report(void);
#endif
