#ifndef PSRE_LIMINE_H
#define PSRE_LIMINE_H
#include <stdint.h>

#define LIMINE_COMMON_MAGIC_0 0xc7b1dd30df4c8b88ULL
#define LIMINE_COMMON_MAGIC_1 0x0a82e883a194f07bULL
#define LIMINE_MEMMAP_REQUEST_ID \
    { LIMINE_COMMON_MAGIC_0, LIMINE_COMMON_MAGIC_1, 0x67cf3d9d378a806fULL, 0xe304acdfc50c3c62ULL }
#define LIMINE_HHDM_REQUEST_ID \
    { LIMINE_COMMON_MAGIC_0, LIMINE_COMMON_MAGIC_1, 0x48dcf1cb8ad2b852ULL, 0x63984e959a98244bULL }
#define LIMINE_RSDP_REQUEST_ID \
    { LIMINE_COMMON_MAGIC_0, LIMINE_COMMON_MAGIC_1, 0xc5e77b6b397e7b43ULL, 0x27637845accdcf3cULL }
#define LIMINE_FRAMEBUFFER_REQUEST_ID \
    { LIMINE_COMMON_MAGIC_0, LIMINE_COMMON_MAGIC_1, \
      0x9d5827dcd881dd75ULL, 0xa3148604f6fab11bULL }
#define LIMINE_FRAMEBUFFER_REQUEST_REVISION 0ULL

#define LIMINE_MEMMAP_USABLE 0
#define LIMINE_MEMMAP_RESERVED 1
#define LIMINE_MEMMAP_ACPI_RECLAIMABLE 2
#define LIMINE_MEMMAP_ACPI_NVS 3
#define LIMINE_MEMMAP_BAD_MEMORY 4
#define LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE 5
#define LIMINE_MEMMAP_EXECUTABLE_AND_MODULES 6
#define LIMINE_MEMMAP_FRAMEBUFFER 7
#define LIMINE_MEMMAP_RESERVED_MAPPED 8

struct limine_memmap_entry { uint64_t base, length, type; };
struct limine_memmap_response {
    uint64_t revision;
    uint64_t entry_count;
    struct limine_memmap_entry **entries;
};
struct limine_memmap_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_memmap_response *response;
};
struct limine_hhdm_response { uint64_t revision, offset; };
struct limine_rsdp_response { uint64_t revision; void *address; };
struct limine_hhdm_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_hhdm_response *response;
};

extern volatile struct limine_memmap_request psre_memmap_request;
extern volatile struct limine_hhdm_request psre_hhdm_request;
struct limine_rsdp_request { uint64_t id[4]; uint64_t revision; struct limine_rsdp_response *response; };
extern volatile struct limine_rsdp_request psre_rsdp_request;

#define LIMINE_FRAMEBUFFER_MODEL_TEXT 0
#define LIMINE_FRAMEBUFFER_MODEL_RGB 1
struct limine_framebuffer {
    void *address;
    uint64_t width, height, pitch;
    uint16_t bpp;
    uint8_t memory_model;
    uint8_t red_mask_size, red_mask_shift;
    uint8_t green_mask_size, green_mask_shift;
    uint8_t blue_mask_size, blue_mask_shift;
    uint8_t unused_mask_size, unused_mask_shift;
    uint64_t edid_size;
    void *edid;
};
struct limine_framebuffer_response { uint64_t revision, framebuffer_count; struct limine_framebuffer **framebuffers; };
struct limine_framebuffer_request { uint64_t id[4], revision; struct limine_framebuffer_response *response; };
extern volatile struct limine_framebuffer_request psre_framebuffer_request;

/* Limine protocol ABI: request ID (32 bytes), revision (8), response (8). */
_Static_assert(sizeof(struct limine_framebuffer_request) == 48,
               "Limine framebuffer request ABI size mismatch");
_Static_assert(__builtin_offsetof(struct limine_framebuffer_request, response) == 40,
               "Limine framebuffer response pointer offset mismatch");
#endif
