#include <psre/limine.h>

/*
 * Limine request marker block.
 * The linker script keeps these input sections in one contiguous,
 * writable .limine_requests output section in the exact marker order.
 */
__attribute__((used, aligned(8), section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[4] = {
    0xf6b8f4b39de7d1aeULL, 0xfab91a6940fcb9cfULL,
    0x785c6ed015d3e316ULL, 0x181e920a7852b9d9ULL
};

__attribute__((used, aligned(8), section(".limine_requests")))
static volatile uint64_t limine_base_revision[3] = {
    0xf9562b2d5c95a6c8ULL, 0x6a7b384944536bdcULL, 4
};

__attribute__((used, aligned(8), section(".limine_requests")))
volatile struct limine_memmap_request psre_memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID, .revision = 0, .response = 0
};

__attribute__((used, aligned(8), section(".limine_requests")))
volatile struct limine_hhdm_request psre_hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID, .revision = 0, .response = 0
};

__attribute__((used, aligned(8), section(".limine_requests")))
volatile struct limine_rsdp_request psre_rsdp_request = {
    .id = LIMINE_RSDP_REQUEST_ID, .revision = 0, .response = 0
};

/* Limine framebuffer request ID from the protocol ABI; revision 0. */
__attribute__((used, aligned(8), section(".limine_requests")))
volatile struct limine_framebuffer_request psre_framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = LIMINE_FRAMEBUFFER_REQUEST_REVISION,
    .response = 0
};

__attribute__((used, aligned(8), section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[2] = {
    0xadc0e0531bb10d03ULL, 0x9572709f31764c62ULL
};
