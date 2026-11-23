#ifndef PSRE_PMM_H
#define PSRE_PMM_H
#include <stdint.h>
#include <stdbool.h>
#define PSRE_PAGE_SIZE 4096ULL
struct psre_pmm_stats {
    uint64_t total_pages;
    uint64_t free_pages;
    uint64_t allocated_pages;
    uint64_t managed_limit_bytes;
};
bool pmm_initialize(void);
uint64_t pmm_alloc_page(void); /* physical address; 0 means failure */
uint64_t pmm_alloc_contiguous_pages(uint64_t count, uint64_t max_address);
bool pmm_free_contiguous_pages(uint64_t physical_address, uint64_t count);
bool pmm_free_page(uint64_t physical_address);
const struct psre_pmm_stats *pmm_get_stats(void);
#endif
