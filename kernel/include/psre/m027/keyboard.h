#ifndef PSRE_M027_KEYBOARD_H
#define PSRE_M027_KEYBOARD_H
#include <stdbool.h>
#include <stdint.h>
#define PSRE_M027_REPEAT_DELAY_TICKS 50u
#define PSRE_M027_REPEAT_INTERVAL_TICKS 5u
#define PSRE_M027_KEY_BITMAP_BYTES 32u
typedef struct {
    bool initialized;
    uint8_t interface_number, modifiers, leds, repeat_key;
    uint8_t key_bitmap[PSRE_M027_KEY_BITMAP_BYTES];
    uint32_t reports, rollover_reports, repeat_events;
    uint32_t led_updates, led_errors;
    uint64_t repeat_deadline;
} psre_m027_status_t;
/* M027 extends Boot Keyboard handling: full 256-usage state bitmap, repeat,
   and HID Output Report LED control. It remains Boot Protocol (6KRO). */
bool psre_m027_initialize(uint8_t interface_number);
bool psre_m027_process_report(const uint8_t *report, uint16_t length,
                              uint64_t now_ticks, uint8_t *pressed_key);
bool psre_m027_repeat_due(uint64_t now_ticks, uint8_t *key);
void psre_m027_report(void);
const psre_m027_status_t *psre_m027_get_status(void);
#endif
