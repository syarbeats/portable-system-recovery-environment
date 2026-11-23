#ifndef PSRE_M004_ACPI_H
#define PSRE_M004_ACPI_H
#include <stdint.h>
#include <stdbool.h>
typedef struct { char signature[4]; uint32_t length; uint8_t revision, checksum; char oem_id[6], oem_table_id[8]; uint32_t oem_revision, creator_id, creator_revision; } __attribute__((packed)) psre_acpi_sdt_header_t;
typedef struct { uint32_t table_count, madt_count, mcfg_count, facp_count, hpet_count, other_count, local_apic_count, io_apic_count; bool rsdp_found, xsdt_used, checksum_valid; } psre_acpi_stats_t;
bool psre_acpi_initialize(void);
const psre_acpi_stats_t *psre_acpi_get_stats(void);
void psre_acpi_report(void);
#endif

