#ifndef PSRE_M005_FAT32_H
#define PSRE_M005_FAT32_H
#include <stdbool.h>
#include <stdint.h>
typedef struct { bool valid; uint8_t device_index; uint64_t partition_lba; uint32_t sectors_per_cluster, reserved_sectors, fat_count, sectors_per_fat, root_cluster; uint64_t total_sectors; } psre_fat32_volume_t;
bool psre_fat32_probe(uint8_t device_index, uint64_t lba, psre_fat32_volume_t *volume);
bool psre_fat32_probe_usb(uint64_t lba, psre_fat32_volume_t *volume);
void psre_fat32_report(const psre_fat32_volume_t *volume);
#endif
