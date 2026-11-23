#include <psre/limine.h>
#include <psre/m031/framebuffer.h>
#include <psre/m032/graphics.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
volatile struct limine_framebuffer_request psre_framebuffer_request;
void serial_write(const char *s) {(void)s;}
void serial_write_char(char c) {(void)c;}
/* M031 back buffer deps (not used by this test) */
uint64_t pmm_alloc_contiguous_pages(uint64_t c,uint64_t m){(void)c;(void)m;return 0;}
uint64_t memory_map_hhdm_offset(void){return 0;}
static unsigned char fbmem[64u*48u*4u];
static struct limine_framebuffer fb;
static struct limine_framebuffer *fbptr = &fb;
static struct limine_framebuffer_response response;
int main(void) {
    memset(fbmem,0xA5,sizeof(fbmem));
    fb.address=fbmem; fb.width=64; fb.height=48; fb.pitch=64*4; fb.bpp=32;
    fb.memory_model=LIMINE_FRAMEBUFFER_MODEL_RGB;
    fb.red_mask_size=8; fb.red_mask_shift=16;
    fb.green_mask_size=8; fb.green_mask_shift=8;
    fb.blue_mask_size=8; fb.blue_mask_shift=0;
    response.framebuffer_count=1; response.framebuffers=&fbptr;
    psre_framebuffer_request.response=&response;
    assert(psre_m031_initialize());
    for(unsigned i=0;i<sizeof(fbmem);++i) assert(fbmem[i]==0xA5); /* init does not draw */
    assert(psre_m032_initialize());
    psre_m032_color_t red={255,0,0}, green={0,255,0}, blue={0,0,255}, white={255,255,255};
    assert(psre_m032_draw_pixel(2,3,red));
    unsigned off=(3*64+2)*4;
    assert(fbmem[off]==0 && fbmem[off+1]==0 && fbmem[off+2]==255);
    assert(!psre_m032_draw_pixel(-1,0,red));
    assert(psre_m032_fill_rect(-3,-2,6,5,green));
    assert(fbmem[0]==0 && fbmem[1]==255 && fbmem[2]==0);
    assert(psre_m032_draw_line(-10,10,70,10,blue));
    assert(fbmem[(10*64)*4]==255 && fbmem[(10*64+63)*4]==255);
    assert(psre_m032_draw_rect(4,4,10,8,white));
    assert(psre_m032_fill_rect(20,5,8,7,red));
    assert(psre_m032_draw_circle(32,24,8,white));
    assert(psre_m032_fill_circle(48,24,5,green));
    assert(psre_m032_fill_triangle(3,40,20,30,30,44,blue));
    assert(psre_m032_fill_triangle(3,40,30,44,20,30,blue));
    assert(!psre_m032_fill_triangle(1,1,2,2,3,3,red));
    unsigned char bmp[]={255,0,0, 0,255,0, 0,0,255, 255,255,255};
    psre_m032_bitmap_t image={2,2,6,bmp,PSRE_M032_BITMAP_RGB24};
    assert(psre_m032_draw_bitmap(62,46,&image));
    assert(!psre_m032_draw_bitmap(0,0,NULL));
    assert(!psre_m032_draw_bitmap(0,0,&(psre_m032_bitmap_t){2,2,5,bmp,PSRE_M032_BITMAP_RGB24}));
    psre_m032_bitmap_t bgr_image={2,2,6,bmp,PSRE_M032_BITMAP_BGR24};
    assert(psre_m032_draw_bitmap(5,5,&bgr_image));
    unsigned bgr_off=(5u*64u+5u)*4u;
    assert(fbmem[bgr_off]==255 && fbmem[bgr_off+2]==0);
    assert(psre_m032_draw_demo());
    /* Reinitialize M031 with 24-bpp RGB and a padded pitch. */
    static unsigned char fb24[(64u*3u+8u)*48u];
    memset(fb24,0x5A,sizeof(fb24));
    fb.address=fb24; fb.width=64; fb.height=48; fb.pitch=64*3+8; fb.bpp=24;
    assert(psre_m031_initialize());
    assert(psre_m032_initialize());
    assert(psre_m032_draw_pixel(1,1,red));
    unsigned off24=(unsigned)fb.pitch+3u;
    assert(fb24[off24]==0 && fb24[off24+1]==0 && fb24[off24+2]==255);
    assert(fb24[64u*3u+7u]==0x5A); /* row padding untouched */
    puts("M032_SYNTHETIC_TEST_PASS");
    return 0;
}
