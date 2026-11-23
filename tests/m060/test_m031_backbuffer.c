/* Host regression: with the back buffer enabled, drawing must NOT touch the visible
   framebuffer until psre_m031_present(); present copies only dirty rows. */
#include <psre/m031/framebuffer.h>
#include <psre/limine.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define W 64
#define H 32
static uint8_t *vram; static uint8_t *pool;
volatile struct limine_framebuffer_request psre_framebuffer_request;
void serial_write(const char *s){(void)s;} void serial_write_char(char c){(void)c;}
uint64_t memory_map_hhdm_offset(void){return (uint64_t)(uintptr_t)pool-4096u;}
uint64_t pmm_alloc_contiguous_pages(uint64_t c,uint64_t m){(void)c;(void)m;return 4096u;}
#define CHECK(c) do{if(!(c)){printf("FAIL line %d: %s\n",__LINE__,#c);return 1;}}while(0)
static uint32_t px(uint64_t x,uint64_t y){uint32_t v;memcpy(&v,vram+y*(W*4)+x*4,4);return v;}
int main(void){
    vram=calloc(1,W*H*4); pool=calloc(1,W*H*4+4096);
    static struct limine_framebuffer f; static struct limine_framebuffer *fp[1]; static struct limine_framebuffer_response r;
    f=(struct limine_framebuffer){.address=vram,.width=W,.height=H,.pitch=W*4,.bpp=32,.memory_model=LIMINE_FRAMEBUFFER_MODEL_RGB,.red_mask_size=8,.red_mask_shift=16,.green_mask_size=8,.green_mask_shift=8,.blue_mask_size=8,.blue_mask_shift=0};
    fp[0]=&f; r=(struct limine_framebuffer_response){1,1,fp}; psre_framebuffer_request.response=&r;
    CHECK(psre_m031_initialize());
    /* direct mode still works */
    CHECK(psre_m031_fill_rect(0,0,W,H,10,20,30)); CHECK(px(5,5)==((10u<<16)|(20u<<8)|30u));
    CHECK(psre_m031_enable_backbuffer()); CHECK(psre_m031_backbuffer_enabled());
    /* draw: visible fb unchanged */
    CHECK(psre_m031_fill_rect(0,0,W,H,255,0,0));
    CHECK(px(5,5)==((10u<<16)|(20u<<8)|30u));
    CHECK(psre_m031_put_pixel(7,3,0,255,0));
    CHECK(px(7,3)==((10u<<16)|(20u<<8)|30u));
    /* present: now visible, and only if dirty */
    CHECK(psre_m031_present()); CHECK(px(5,5)==(255u<<16)); CHECK(px(7,3)==(255u<<8));
    CHECK(!psre_m031_present());
    /* partial update touches only its rows */
    memset(vram,0xAB,W*H*4);                 /* poison visible fb */
    CHECK(psre_m031_put_pixel(1,10,0,0,255)); CHECK(psre_m031_present());
    CHECK(px(1,10)==255u); CHECK(px(0,9)==0xABABABABu); CHECK(px(0,11)==0xABABABABu);
    puts("M031 double-buffer host test: PASS"); return 0;
}
