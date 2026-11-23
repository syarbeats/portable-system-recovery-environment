#ifndef PSRE_M006_FS_H
#define PSRE_M006_FS_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <psre/m005/fat32.h>
#define PSRE_FS_NAME_MAX 13
#define PSRE_FS_MAX_PATH 128
bool psre_fat32_mount_first(void);
bool psre_fat32_mount_usb(void);
bool psre_fat32_is_mounted(void);
const psre_fat32_volume_t *psre_fat32_get_mounted(void);
/* Prints precise diagnostics on failure; returns true on a complete listing. */
bool psre_fat32_list_root(void);
/* Reads a root-directory 8.3 filename; read-only. */
bool psre_fat32_cat_root(const char *short_name);
/* M007 read-only path operations; path components use FAT 8.3 names. */
bool psre_m007_change_dir(const char *path);
const char *psre_m007_get_cwd(void);
bool psre_m007_list_dir(const char *path);
#define PSRE_M007_MAX_DIRECTORY_ENTRIES 64u
typedef struct {
    char name[14];
    uint32_t size;
    uint32_t first_cluster;
    uint8_t attributes;
} psre_m007_dir_entry_t;
typedef struct {
    char path[PSRE_FS_MAX_PATH];
    uint32_t size;
    uint32_t crc32;
} psre_m009_queue_entry_t;
bool psre_m007_read_dir(const char *path, psre_m007_dir_entry_t *entries,
                        uint32_t capacity, uint32_t *count, uint32_t *total);
bool psre_m009_queue_get_info(uint32_t *count, uint32_t *used_bytes);
bool psre_m009_queue_get_entry(uint32_t index, psre_m009_queue_entry_t *out);

bool psre_m007_cat_path(const char *path);
bool psre_m007_stat(const char *path);
bool psre_m007_crc32(const char *path);
/* M008: bounded RAM recovery workspace; source volume remains read-only. */
bool psre_m008_recover(const char *path);
void psre_m008_workspace_info(void);
void psre_m008_manifest(void);
bool psre_m008_export_serial(void);
void psre_m008_clear(void);
/* M009: multi-file RAM recovery queue (bounded; source remains read-only). */
bool psre_m009_queue_add(const char *path);
void psre_m009_queue_list(void);
void psre_m009_queue_manifest(void);
bool psre_m009_queue_export(void);
/* M043 bounded, cooperative serial export. */
bool psre_m009_export_begin(void);
bool psre_m009_export_step(uint32_t byte_budget, uint32_t *done, uint32_t *total, uint32_t *files_done, bool *complete);
void psre_m009_export_cancel(void);
void psre_m009_queue_clear(void);
void psre_m006_initialize(void);
/* Execute one CLI command line; output is written via serial_write (tap with serial_set_sink). */
void psre_m006_execute(const char *line);
#endif
