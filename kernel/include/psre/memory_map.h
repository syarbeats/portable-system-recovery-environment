#ifndef PSRE_MEMORY_MAP_H
#define PSRE_MEMORY_MAP_H
#include <stdint.h>
#include <stdbool.h>
#include <psre/limine.h>

struct psre_memory_map_stats {
    uint64_t entry_count;
    uint64_t usable_bytes;
    uint64_t reserved_bytes;
    uint64_t highest_address;
};

bool memory_map_initialize(void);
const struct limine_memmap_response *memory_map_get(void);
uint64_t memory_map_hhdm_offset(void);
const struct psre_memory_map_stats *memory_map_get_stats(void);
#endif
