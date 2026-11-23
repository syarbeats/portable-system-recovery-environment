#ifndef PSRE_M029_LAYOUT_H
#define PSRE_M029_LAYOUT_H
#include <stdint.h>
#include <stdbool.h>
#define PSRE_M029_LAYOUT_US_QWERTY 1u
#define PSRE_M029_MOD_LSHIFT 0x02u
#define PSRE_M029_MOD_RSHIFT 0x20u
#define PSRE_M029_MOD_LCTRL 0x01u
#define PSRE_M029_MOD_RCTRL 0x10u
#define PSRE_M029_LED_CAPS_LOCK 0x01u
/* Returns printable ASCII or the documented control character; 0 means no character. */
char psre_m029_translate(uint8_t usage, uint8_t modifiers, uint8_t led_mask);
const char *psre_m029_layout_name(void);
void psre_m029_report(void);
#endif
