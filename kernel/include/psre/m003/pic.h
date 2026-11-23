#ifndef PSRE_M003_PIC_H
#define PSRE_M003_PIC_H
#include <stdint.h>
void psre_pic_initialize(void);
void psre_pic_set_mask(uint8_t irq);
void psre_pic_clear_mask(uint8_t irq);
void psre_pic_send_eoi(uint8_t irq);
#endif
