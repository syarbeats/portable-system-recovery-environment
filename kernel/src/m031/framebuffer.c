#include <psre/m031/framebuffer.h>
#include <psre/limine.h>
#include <psre/serial.h>
#include <psre/pmm.h>
#include <psre/memory_map.h>
#define PSRE_M031_MAX_FRAMEBUFFER_BYTES (1024ULL * 1024ULL * 1024ULL)
static psre_m031_framebuffer_t g_fb;
static psre_m031_error_t g_error = PSRE_M031_ERROR_NONE;
static uint64_t g_response_revision, g_response_count;
static bool mask_valid(uint8_t size, uint8_t shift, uint16_t bpp) {
    return size > 0u && size <= 8u && shift < bpp && (uint16_t)size + (uint16_t)shift <= bpp;
}
psre_m031_error_t psre_m031_last_error(void) { return g_error; }
const char *psre_m031_error_string(psre_m031_error_t e) {
    switch (e) {
        case PSRE_M031_ERROR_NONE: return "none";
        case PSRE_M031_ERROR_NO_RESPONSE: return "Limine framebuffer response is NULL";
        case PSRE_M031_ERROR_NO_FRAMEBUFFERS: return "Limine returned zero framebuffers";
        case PSRE_M031_ERROR_BAD_COUNT: return "framebuffer count exceeds safety limit";
        case PSRE_M031_ERROR_NULL_DESCRIPTOR: return "first framebuffer descriptor is NULL";
        case PSRE_M031_ERROR_BAD_GEOMETRY: return "invalid framebuffer address or dimensions";
        case PSRE_M031_ERROR_UNSUPPORTED_BPP: return "unsupported bits-per-pixel (expected 16, 24 or 32)";
        case PSRE_M031_ERROR_UNSUPPORTED_MODEL: return "unsupported framebuffer memory model (expected RGB)";
        case PSRE_M031_ERROR_BAD_PITCH: return "framebuffer pitch is smaller than a scanline";
        case PSRE_M031_ERROR_BAD_SIZE: return "framebuffer byte size is invalid or exceeds 1 GiB";
        case PSRE_M031_ERROR_BAD_MASK: return "invalid RGB channel mask";
        case PSRE_M031_ERROR_OVERLAPPING_MASKS: return "RGB channel masks overlap";
        default: return "unknown framebuffer validation error";
    }
}
bool psre_m031_initialize(void) {
    g_fb = (psre_m031_framebuffer_t){0};
    g_error = PSRE_M031_ERROR_NONE;
    struct limine_framebuffer_response *r = psre_framebuffer_request.response;
    if (!r) { g_error = PSRE_M031_ERROR_NO_RESPONSE; return false; }
    g_response_revision = r->revision;
    g_response_count = r->framebuffer_count;
    if (!r->framebuffers || r->framebuffer_count == 0u) { g_error = PSRE_M031_ERROR_NO_FRAMEBUFFERS; return false; }
    if (r->framebuffer_count > 16u) { g_error = PSRE_M031_ERROR_BAD_COUNT; return false; }
    struct limine_framebuffer *f = r->framebuffers[0];
    if (!f) { g_error = PSRE_M031_ERROR_NULL_DESCRIPTOR; return false; }
    if (!f->address || !f->width || !f->height || f->width > 16384u || f->height > 16384u) {
        g_error = PSRE_M031_ERROR_BAD_GEOMETRY; return false;
    }
    if (f->bpp != 16u && f->bpp != 24u && f->bpp != 32u) { g_error = PSRE_M031_ERROR_UNSUPPORTED_BPP; return false; }
    if (f->memory_model != LIMINE_FRAMEBUFFER_MODEL_RGB) { g_error = PSRE_M031_ERROR_UNSUPPORTED_MODEL; return false; }
    uint8_t bytes = (uint8_t)((f->bpp + 7u) / 8u);
    if (f->width > UINT64_MAX / bytes) { g_error = PSRE_M031_ERROR_BAD_PITCH; return false; }
    uint64_t min_pitch = f->width * bytes;
    if (f->pitch == 0u || f->pitch < min_pitch || f->height > UINT64_MAX / f->pitch) {
        g_error = PSRE_M031_ERROR_BAD_PITCH; return false;
    }
    uint64_t total = f->pitch * f->height;
    if (!total || total > PSRE_M031_MAX_FRAMEBUFFER_BYTES) { g_error = PSRE_M031_ERROR_BAD_SIZE; return false; }
    if (!mask_valid(f->red_mask_size, f->red_mask_shift, f->bpp) ||
        !mask_valid(f->green_mask_size, f->green_mask_shift, f->bpp) ||
        !mask_valid(f->blue_mask_size, f->blue_mask_shift, f->bpp)) { g_error = PSRE_M031_ERROR_BAD_MASK; return false; }
    uint32_t red_mask = ((1u << f->red_mask_size) - 1u) << f->red_mask_shift;
    uint32_t green_mask = ((1u << f->green_mask_size) - 1u) << f->green_mask_shift;
    uint32_t blue_mask = ((1u << f->blue_mask_size) - 1u) << f->blue_mask_shift;
    if ((red_mask & green_mask) || (red_mask & blue_mask) || (green_mask & blue_mask)) {
        g_error = PSRE_M031_ERROR_OVERLAPPING_MASKS; return false;
    }
    g_fb.address=(volatile uint8_t *)f->address; g_fb.byte_size=total;
    g_fb.width=f->width; g_fb.height=f->height; g_fb.pitch=f->pitch;
    g_fb.bits_per_pixel=f->bpp; g_fb.bytes_per_pixel=bytes;
    g_fb.red_mask_size=f->red_mask_size; g_fb.red_mask_shift=f->red_mask_shift;
    g_fb.green_mask_size=f->green_mask_size; g_fb.green_mask_shift=f->green_mask_shift;
    g_fb.blue_mask_size=f->blue_mask_size; g_fb.blue_mask_shift=f->blue_mask_shift;
    g_fb.pixel_model=PSRE_M031_PIXEL_RGB; g_fb.available=true;
    return true;
}
const psre_m031_framebuffer_t *psre_m031_get(void) { return &g_fb; }
static uint32_t channel_value(uint8_t value, uint8_t size, uint8_t shift) {
    uint32_t max=(1u<<size)-1u; return ((((uint32_t)value*max+127u)/255u)<<shift);
}
/* ---- back buffer -------------------------------------------------------- */
static uint8_t *g_back;
static bool g_dirty;
static uint64_t g_dirty_y0, g_dirty_y1;
static volatile uint8_t *draw_target(void) { return g_back ? (volatile uint8_t *)g_back : g_fb.address; }
static void mark_dirty(uint64_t y0, uint64_t y1) {
    if(!g_back) return;
    if(!g_dirty){ g_dirty=true; g_dirty_y0=y0; g_dirty_y1=y1; return; }
    if(y0<g_dirty_y0) g_dirty_y0=y0;
    if(y1>g_dirty_y1) g_dirty_y1=y1;
}
static void copy_bytes(volatile uint8_t *dst, const volatile uint8_t *src, uint64_t n) {
    uint64_t q=n>>3, r=n&7u;
    __asm__ volatile("rep movsq" : "+D"(dst), "+S"(src), "+c"(q) : : "memory");
    __asm__ volatile("rep movsb" : "+D"(dst), "+S"(src), "+c"(r) : : "memory");
}
bool psre_m031_enable_backbuffer(void) {
    if(!g_fb.available) return false;
    if(g_back) return true;
    uint64_t pages=(g_fb.byte_size+PSRE_PAGE_SIZE-1u)/PSRE_PAGE_SIZE;
    uint64_t phys=pmm_alloc_contiguous_pages(pages,~0ULL);
    if(!phys){ serial_write("[M031] Back buffer allocation failed; drawing directly (may flicker).\n"); return false; }
    g_back=(uint8_t *)(uintptr_t)(memory_map_hhdm_offset()+phys);
    copy_bytes((volatile uint8_t *)g_back,g_fb.address,g_fb.byte_size); /* keep current screen contents */
    g_dirty=false;
    serial_write("[M031] Double buffering enabled.\n");
    return true;
}
bool psre_m031_backbuffer_enabled(void) { return g_back!=0; }
bool psre_m031_present(void) {
    if(!g_back||!g_dirty) return false;
    uint64_t y0=g_dirty_y0, y1=g_dirty_y1;
    if(y1>=g_fb.height) y1=g_fb.height-1u;
    uint64_t row_bytes=g_fb.width*g_fb.bytes_per_pixel;
    for(uint64_t y=y0;y<=y1;++y)
        copy_bytes(g_fb.address+y*g_fb.pitch,g_back+y*g_fb.pitch,row_bytes);
    g_dirty=false;
    return true;
}
static uint32_t pack_pixel(uint8_t red,uint8_t green,uint8_t blue) {
    return channel_value(red,g_fb.red_mask_size,g_fb.red_mask_shift)|
           channel_value(green,g_fb.green_mask_size,g_fb.green_mask_shift)|
           channel_value(blue,g_fb.blue_mask_size,g_fb.blue_mask_shift);
}
bool psre_m031_put_pixel(uint64_t x,uint64_t y,uint8_t red,uint8_t green,uint8_t blue) {
    if(!g_fb.available||x>=g_fb.width||y>=g_fb.height) return false;
    uint64_t off=y*g_fb.pitch+x*g_fb.bytes_per_pixel;
    if(off>g_fb.byte_size||g_fb.bytes_per_pixel>g_fb.byte_size-off) return false;
    uint32_t px=pack_pixel(red,green,blue);
    volatile uint8_t *t=draw_target();
    for(uint8_t i=0;i<g_fb.bytes_per_pixel;++i) t[off+i]=(uint8_t)(px>>(8u*i));
    mark_dirty(y,y);
    return true;
}
bool psre_m031_fill_rect(uint64_t x,uint64_t y,uint64_t w,uint64_t h,uint8_t r,uint8_t g,uint8_t b) {
    if(!g_fb.available||!w||!h||x>=g_fb.width||y>=g_fb.height) return false;
    uint64_t right=w>g_fb.width-x?g_fb.width:x+w;
    uint64_t bottom=h>g_fb.height-y?g_fb.height:y+h;
    uint32_t px=pack_pixel(r,g,b);
    uint8_t bpp=g_fb.bytes_per_pixel;
    volatile uint8_t *t=draw_target();
    for(uint64_t py=y;py<bottom;++py) {
        volatile uint8_t *row=t+py*g_fb.pitch+x*bpp;
        if(bpp==4u) { volatile uint32_t *d=(volatile uint32_t *)row; for(uint64_t n=right-x;n;--n) *d++=px; }
        else for(uint64_t n=right-x;n;--n) { for(uint8_t i=0;i<bpp;++i) row[i]=(uint8_t)(px>>(8u*i)); row+=bpp; }
    }
    mark_dirty(y,bottom-1u);
    return true;
}
bool psre_m031_draw_test_pattern(void) {
    if(!g_fb.available) return false;
    uint64_t bar=g_fb.width/3u; if(!bar) return false;
    bool ok=psre_m031_fill_rect(0,0,bar,g_fb.height,255,0,0);
    ok=psre_m031_fill_rect(bar,0,bar,g_fb.height,0,255,0)&&ok;
    ok=psre_m031_fill_rect(bar*2u,0,g_fb.width-bar*2u,g_fb.height,0,0,255)&&ok;
    return ok;
}
static void print_u64(uint64_t v) { char b[21]; unsigned n=0; do { b[n++]=(char)('0'+v%10u); v/=10u; } while(v&&n<sizeof(b)); while(n) serial_write_char(b[--n]); }
void psre_m031_report(void) {
    serial_write("[M031] Limine framebuffer diagnostics\n[M031] Response: ");
    if (!psre_framebuffer_request.response) {
        serial_write("NULL\n[M031] Result: "); serial_write(psre_m031_error_string(g_error)); serial_write("\n"); return;
    }
    serial_write("present | revision: "); print_u64(g_response_revision);
    serial_write(" | count: "); print_u64(g_response_count); serial_write("\n");
    if (!g_fb.available) {
        serial_write("[M031] Validation rejected framebuffer: ");
        serial_write(psre_m031_error_string(g_error)); serial_write("\n");
        struct limine_framebuffer_response *r = psre_framebuffer_request.response;
        if (r->framebuffers && r->framebuffer_count > 0u && r->framebuffer_count <= 16u && r->framebuffers[0]) {
            struct limine_framebuffer *f = r->framebuffers[0];
            serial_write("[M031] Raw descriptor: width="); print_u64(f->width);
            serial_write(" height="); print_u64(f->height);
            serial_write(" pitch="); print_u64(f->pitch);
            serial_write(" bpp="); print_u64(f->bpp);
            serial_write(" model="); print_u64(f->memory_model); serial_write("\n");
        }
        return;
    }
    serial_write("[M031] Framebuffer available (Limine virtual address).\n[M031] Resolution: ");
    print_u64(g_fb.width);serial_write("x");print_u64(g_fb.height);
    serial_write(" | Pitch: ");print_u64(g_fb.pitch);serial_write(" | BPP: ");print_u64(g_fb.bits_per_pixel);
    serial_write(" | Bytes/pixel: ");print_u64(g_fb.bytes_per_pixel);
    serial_write(" | RGB masks R=");print_u64(g_fb.red_mask_size);serial_write("@");print_u64(g_fb.red_mask_shift);
    serial_write(" G=");print_u64(g_fb.green_mask_size);serial_write("@");print_u64(g_fb.green_mask_shift);
    serial_write(" B=");print_u64(g_fb.blue_mask_size);serial_write("@");print_u64(g_fb.blue_mask_shift);serial_write("\n");
}
