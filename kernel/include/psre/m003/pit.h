#ifndef PSRE_M003_PIT_H
#define PSRE_M003_PIT_H
#include <stdint.h>
void psre_pit_initialize(uint32_t frequency_hz);
uint64_t psre_pit_ticks(void);
#endif
