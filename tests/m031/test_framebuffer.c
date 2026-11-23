#include <psre/limine.h>
#include <psre/m031/framebuffer.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
volatile struct limine_framebuffer_request psre_framebuffer_request;
void serial_write(const char *s) {(void)s;}
void serial_write_char(char c) {(void)c;}
/* M031 back buffer deps (not used by this test) */
uint64_t pmm_alloc_contiguous_pages(uint64_t c,uint64_t m){(void)c;(void)m;return 0;}
uint64_t memory_map_hhdm_offset(void){return 0;}
static unsigned char pixels[64u*48u*4u];
static struct limine_framebuffer fb;
static struct limine_framebuffer *fbptr=&fb;
static struct limine_framebuffer_response response;
static void setup(unsigned bpp) {
    memset(&fb,0,sizeof(fb)); memset(pixels,0,sizeof(pixels));
    fb.address=pixels; fb.width=64; fb.height=48; fb.bpp=(uint16_t)bpp;
    fb.memory_model=LIMINE_FRAMEBUFFER_MODEL_RGB;
    if (bpp==32 || bpp==24) {
        fb.pitch=64u*(bpp/8u); fb.red_mask_size=8; fb.red_mask_shift=16;
        fb.green_mask_size=8; fb.green_mask_shift=8; fb.blue_mask_size=8; fb.blue_mask_shift=0;
    } else {
        fb.pitch=64u*2u; fb.red_mask_size=5; fb.red_mask_shift=11;
        fb.green_mask_size=6; fb.green_mask_shift=5; fb.blue_mask_size=5; fb.blue_mask_shift=0;
    }
    response.revision=0; response.framebuffer_count=1; response.framebuffers=&fbptr;
    psre_framebuffer_request.response=&response;
}
int main(void) {
    psre_framebuffer_request.response=NULL;
    assert(!psre_m031_initialize());
    assert(psre_m031_last_error()==PSRE_M031_ERROR_NO_RESPONSE);
    setup(32); assert(psre_m031_initialize()); assert(psre_m031_last_error()==PSRE_M031_ERROR_NONE);
    assert(psre_m031_put_pixel(1,1,255,0,0));
    assert(pixels[(1u*64u+1u)*4u+2u]==255u);
    assert(psre_m031_draw_test_pattern());
    setup(24); assert(psre_m031_initialize()); assert(psre_m031_put_pixel(2,2,0,255,0));
    assert(pixels[(2u*64u+2u)*3u+1u]==255u);
    setup(16); assert(psre_m031_initialize()); assert(psre_m031_put_pixel(3,3,255,0,0));
    assert(pixels[(3u*64u+3u)*2u]==0x00u && pixels[(3u*64u+3u)*2u+1u]==0xF8u);
    setup(8); assert(!psre_m031_initialize());
    assert(psre_m031_last_error()==PSRE_M031_ERROR_UNSUPPORTED_BPP);
    setup(32); fb.pitch=1; assert(!psre_m031_initialize());
    assert(psre_m031_last_error()==PSRE_M031_ERROR_BAD_PITCH);
    puts("M031_FRAMEBUFFER_DIAGNOSTICS_PASS");
    return 0;
}
