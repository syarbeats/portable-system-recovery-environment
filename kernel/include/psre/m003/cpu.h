#ifndef PSRE_M003_CPU_H
#define PSRE_M003_CPU_H

#include <stdint.h>

typedef struct {
    char vendor[13];
    char brand[49];
    uint32_t max_basic_leaf;
    uint32_t max_extended_leaf;
    uint32_t family;
    uint32_t model;
    uint32_t stepping;
    uint8_t has_apic;
    uint8_t has_msr;
    uint8_t has_x2apic;
    uint8_t has_sse;
    uint8_t has_sse2;
} psre_cpu_info_t;

void psre_cpu_detect(psre_cpu_info_t *info);
void psre_cpu_report(void);

#endif
