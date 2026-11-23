#ifndef PSRE_M003_PANIC_H
#define PSRE_M003_PANIC_H
#include <stdint.h>
#include "interrupts.h"
__attribute__((noreturn)) void psre_kernel_panic(const char *reason);
__attribute__((noreturn)) void psre_exception_panic(psre_interrupt_frame_t *frame);
#endif
