#ifndef PSRE_M005_STORAGE_H
#define PSRE_M005_STORAGE_H
#include <stdbool.h>
#include <stdint.h>
#define PSRE_STORAGE_MAX_DEVICES 4
#define PSRE_STORAGE_SECTOR_SIZE 512u
typedef struct { bool present; uint8_t channel, drive; uint16_t signature, capabilities; uint32_t command_sets; uint64_t sectors; char model[41]; } psre_ata_device_t;
typedef struct { uint32_t detected_devices, readable_devices, fat32_volumes; } psre_storage_stats_t;
bool psre_storage_detect_all(void);
const psre_ata_device_t *psre_storage_get_devices(void);
const psre_storage_stats_t *psre_storage_get_stats(void);
bool psre_ata_read_sector(uint8_t device_index, uint64_t lba, void *buffer);
void psre_storage_record_readable(void);
void psre_storage_record_fat32(void);
void psre_storage_report(void);
#endif
