#ifndef PSRE_M031_FRAMEBUFFER_H
#define PSRE_M031_FRAMEBUFFER_H
#include <stdbool.h>
#include <stdint.h>
typedef enum { PSRE_M031_PIXEL_UNKNOWN = 0, PSRE_M031_PIXEL_RGB = 1 } psre_m031_pixel_model_t;
typedef enum {
    PSRE_M031_ERROR_NONE = 0,
    PSRE_M031_ERROR_NO_RESPONSE,
    PSRE_M031_ERROR_NO_FRAMEBUFFERS,
    PSRE_M031_ERROR_BAD_COUNT,
    PSRE_M031_ERROR_NULL_DESCRIPTOR,
    PSRE_M031_ERROR_BAD_GEOMETRY,
    PSRE_M031_ERROR_UNSUPPORTED_BPP,
    PSRE_M031_ERROR_UNSUPPORTED_MODEL,
    PSRE_M031_ERROR_BAD_PITCH,
    PSRE_M031_ERROR_BAD_SIZE,
    PSRE_M031_ERROR_BAD_MASK,
    PSRE_M031_ERROR_OVERLAPPING_MASKS
} psre_m031_error_t;
typedef struct {
    bool available;
    volatile uint8_t *address;
    uint64_t byte_size, width, height, pitch;
    uint16_t bits_per_pixel;
    uint8_t bytes_per_pixel;
    uint8_t red_mask_size, red_mask_shift, green_mask_size, green_mask_shift;
    uint8_t blue_mask_size, blue_mask_shift;
    psre_m031_pixel_model_t pixel_model;
} psre_m031_framebuffer_t;
bool psre_m031_initialize(void);
psre_m031_error_t psre_m031_last_error(void);
const char *psre_m031_error_string(psre_m031_error_t error);
const psre_m031_framebuffer_t *psre_m031_get(void);
bool psre_m031_put_pixel(uint64_t x, uint64_t y, uint8_t red, uint8_t green, uint8_t blue);
bool psre_m031_fill_rect(uint64_t x, uint64_t y, uint64_t width, uint64_t height, uint8_t red, uint8_t green, uint8_t blue);
bool psre_m031_draw_test_pattern(void);
/* Double buffering: after enable, all drawing goes to an off-screen buffer and only
   psre_m031_present() touches the visible framebuffer (flicker-free desktop). */
bool psre_m031_enable_backbuffer(void);
bool psre_m031_backbuffer_enabled(void);
bool psre_m031_present(void);
void psre_m031_report(void);
#endif
