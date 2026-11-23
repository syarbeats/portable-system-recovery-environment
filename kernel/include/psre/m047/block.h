#ifndef PSRE_M047_BLOCK_H
#define PSRE_M047_BLOCK_H
#include <stdbool.h>
#include <stdint.h>
#define PSRE_M047_SECTOR_SIZE 512u
#define PSRE_M047_MAX_READ_SECTORS 8u
typedef struct {
 bool initialized, present, ready;
 uint64_t sectors;
 uint32_t block_size, read_commands, sectors_read, errors;
 uint8_t peripheral_type, removable, lun;
 char vendor[9], product[17], revision[5];
} psre_m047_status_t;
bool psre_m047_initialize(uint8_t lun);
bool psre_m047_read_sectors(uint64_t lba, uint32_t count, void *buffer);
const psre_m047_status_t *psre_m047_get_status(void);
void psre_m047_report(void);
#endif
