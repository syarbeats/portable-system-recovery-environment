#ifndef PSRE_VMM_H
#define PSRE_VMM_H
#include <stdint.h>
#include <stdbool.h>
bool vmm_initialize(void);
uint64_t vmm_get_cr3(void);
/* Returns physical address for a mapped virtual address, or 0 if unmapped. */
uint64_t vmm_translate(uint64_t virtual_address);
/* M013: maps PCI/device MMIO as 4 KiB UC pages in a reserved kernel VA window. */
void *vmm_map_mmio(uint64_t physical_address, uint64_t size);
bool vmm_unmap_mmio(void *virtual_address, uint64_t size);
uint32_t vmm_mmio_active_mappings(void);
#endif
