#include <psre/m032/graphics.h>
#include <psre/m031/framebuffer.h>
#include <psre/serial.h>
#include <limits.h>

#define M032_COORD_LIMIT 16384
static bool g_initialized;
static uint64_t g_operations;
static const psre_m032_color_t C_BLACK = {0,0,0};
static const psre_m032_color_t C_WHITE = {255,255,255};
static const psre_m032_color_t C_RED = {220,35,45};
static const psre_m032_color_t C_GREEN = {35,190,90};
static const psre_m032_color_t C_BLUE = {45,100,220};
static const psre_m032_color_t C_YELLOW = {245,205,55};

static bool coord_ok(int32_t v) { return v >= -M032_COORD_LIMIT && v <= M032_COORD_LIMIT; }
static bool ready(void) { return g_initialized && psre_m031_get()->available; }

bool psre_m032_initialize(void) {
    g_initialized = psre_m031_get()->available;
    g_operations = 0;
    return g_initialized;
}

bool psre_m032_clip_rect(psre_m032_rect_t *r) {
    const psre_m031_framebuffer_t *fb = psre_m031_get();
    if (!r || !fb->available || r->width <= 0 || r->height <= 0) return false;
    int64_t x0 = r->x, y0 = r->y;
    int64_t x1 = x0 + (int64_t)r->width;
    int64_t y1 = y0 + (int64_t)r->height;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > (int64_t)fb->width) x1 = (int64_t)fb->width;
    if (y1 > (int64_t)fb->height) y1 = (int64_t)fb->height;
    if (x0 >= x1 || y0 >= y1) return false;
    r->x = (int32_t)x0; r->y = (int32_t)y0;
    r->width = (int32_t)(x1 - x0); r->height = (int32_t)(y1 - y0);
    return true;
}

bool psre_m032_draw_pixel(int32_t x, int32_t y, psre_m032_color_t c) {
    if (!ready() || x < 0 || y < 0) return false;
    bool ok = psre_m031_put_pixel((uint64_t)x, (uint64_t)y, c.red, c.green, c.blue);
    if (ok) ++g_operations;
    return ok;
}

bool psre_m032_clear(psre_m032_color_t c) {
    if (!ready()) return false;
    bool ok = psre_m031_fill_rect(0, 0, psre_m031_get()->width, psre_m031_get()->height,
                                  c.red, c.green, c.blue);
    if (ok) ++g_operations;
    return ok;
}

bool psre_m032_fill_rect(int32_t x, int32_t y, int32_t w, int32_t h, psre_m032_color_t c) {
    if (!ready()) return false;
    psre_m032_rect_t r = {x,y,w,h};
    if (!psre_m032_clip_rect(&r)) return false;
    bool ok = psre_m031_fill_rect((uint64_t)r.x, (uint64_t)r.y, (uint64_t)r.width,
                                  (uint64_t)r.height, c.red, c.green, c.blue);
    if (ok) ++g_operations;
    return ok;
}

bool psre_m032_draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, psre_m032_color_t c) {
    if (!ready() || !coord_ok(x0) || !coord_ok(y0) || !coord_ok(x1) || !coord_ok(y1)) return false;
    /* Cohen-Sutherland clipping; bounded coordinates keep products within int64_t. */
    const psre_m031_framebuffer_t *fb = psre_m031_get();
    int64_t xmax = (int64_t)fb->width - 1, ymax = (int64_t)fb->height - 1;
    for (unsigned iter=0; iter<8; ++iter) {
        unsigned a = (x0 < 0 ? 1u : x0 > xmax ? 2u : 0u) | (y0 < 0 ? 4u : y0 > ymax ? 8u : 0u);
        unsigned b = (x1 < 0 ? 1u : x1 > xmax ? 2u : 0u) | (y1 < 0 ? 4u : y1 > ymax ? 8u : 0u);
        if (!(a | b)) break;
        if (a & b) return false;
        unsigned out = a ? a : b;
        int64_t x=0,y=0, dx=(int64_t)x1-x0, dy=(int64_t)y1-y0;
        if (out & 8u) { if (!dy) return false; y=ymax; x=x0+dx*(ymax-y0)/dy; }
        else if (out & 4u) { if (!dy) return false; y=0; x=x0+dx*(0-y0)/dy; }
        else if (out & 2u) { if (!dx) return false; x=xmax; y=y0+dy*(xmax-x0)/dx; }
        else { if (!dx) return false; x=0; y=y0+dy*(0-x0)/dx; }
        if (out == a) { x0=(int32_t)x; y0=(int32_t)y; }
        else { x1=(int32_t)x; y1=(int32_t)y; }
    }
    if ((x0 < 0 || x0 > xmax || y0 < 0 || y0 > ymax) ||
        (x1 < 0 || x1 > xmax || y1 < 0 || y1 > ymax)) return false;
    int32_t dx = x1 >= x0 ? x1-x0 : x0-x1;
    int32_t sx = x0 < x1 ? 1 : -1;
    int32_t dy0 = y1 >= y0 ? y1-y0 : y0-y1;
    int32_t dy = -dy0, sy = y0 < y1 ? 1 : -1;
    int32_t err = dx + dy;
    for (;;) {
        (void)psre_m031_put_pixel((uint64_t)x0,(uint64_t)y0,c.red,c.green,c.blue);
        if (x0 == x1 && y0 == y1) break;
        int32_t e2 = 2*err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
    ++g_operations;
    return true;
}

bool psre_m032_draw_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c) {
    if (w <= 0 || h <= 0 || !coord_ok(x) || !coord_ok(y) || !coord_ok(w) || !coord_ok(h)) return false;
    bool a=psre_m032_draw_line(x,y,x+w-1,y,c);
    bool b=psre_m032_draw_line(x,y+h-1,x+w-1,y+h-1,c);
    bool d=psre_m032_draw_line(x,y,x,y+h-1,c);
    bool e=psre_m032_draw_line(x+w-1,y,x+w-1,y+h-1,c);
    return a||b||d||e;
}

static bool circle_points(int32_t cx,int32_t cy,int32_t x,int32_t y,psre_m032_color_t c) {
    bool ok=false;
    ok=psre_m032_draw_pixel(cx+x,cy+y,c)||ok; ok=psre_m032_draw_pixel(cx-x,cy+y,c)||ok;
    ok=psre_m032_draw_pixel(cx+x,cy-y,c)||ok; ok=psre_m032_draw_pixel(cx-x,cy-y,c)||ok;
    ok=psre_m032_draw_pixel(cx+y,cy+x,c)||ok; ok=psre_m032_draw_pixel(cx-y,cy+x,c)||ok;
    ok=psre_m032_draw_pixel(cx+y,cy-x,c)||ok; ok=psre_m032_draw_pixel(cx-y,cy-x,c)||ok;
    return ok;
}

bool psre_m032_draw_circle(int32_t cx,int32_t cy,int32_t radius,psre_m032_color_t c) {
    if (!ready() || radius < 0 || radius > M032_COORD_LIMIT || !coord_ok(cx) || !coord_ok(cy)) return false;
    int32_t x=radius,y=0,err=1-radius; bool ok=false;
    while (x>=y) { ok=circle_points(cx,cy,x,y,c)||ok; ++y; if(err<0) err+=2*y+1; else {--x;err+=2*(y-x)+1;} }
    if (ok) ++g_operations;
    return ok;
}

static bool draw_horizontal_span(int32_t x0, int32_t x1, int32_t y, psre_m032_color_t c) {
    const psre_m031_framebuffer_t *fb = psre_m031_get();
    if (y < 0 || (uint64_t)y >= fb->height) return false;
    if (x0 > x1) { int32_t t=x0; x0=x1; x1=t; }
    if (x0 < 0) x0 = 0;
    if (x1 >= (int32_t)fb->width) x1 = (int32_t)fb->width - 1;
    if (x0 > x1) return false;
    return psre_m031_fill_rect((uint64_t)x0, (uint64_t)y, (uint64_t)(x1-x0+1), 1,
                               c.red, c.green, c.blue);
}

bool psre_m032_fill_circle(int32_t cx,int32_t cy,int32_t radius,psre_m032_color_t c) {
    if (!ready() || radius < 0 || radius > M032_COORD_LIMIT || !coord_ok(cx) || !coord_ok(cy)) return false;
    int32_t x=radius,y=0,err=1-radius; bool ok=false;
    while(x>=y) {
        ok=draw_horizontal_span(cx-x,cx+x,cy+y,c)||ok;
        ok=draw_horizontal_span(cx-x,cx+x,cy-y,c)||ok;
        ok=draw_horizontal_span(cx-y,cx+y,cy+x,c)||ok;
        ok=draw_horizontal_span(cx-y,cx+y,cy-x,c)||ok;
        ++y; if(err<0) err+=2*y+1; else {--x;err+=2*(y-x)+1;}
    }
    if (ok) ++g_operations;
    return ok;
}

bool psre_m032_draw_triangle(int32_t x0,int32_t y0,int32_t x1,int32_t y1,int32_t x2,int32_t y2,psre_m032_color_t c) {
    bool a=psre_m032_draw_line(x0,y0,x1,y1,c);
    bool b=psre_m032_draw_line(x1,y1,x2,y2,c);
    bool d=psre_m032_draw_line(x2,y2,x0,y0,c);
    return a||b||d;
}

static int64_t edge(int32_t ax,int32_t ay,int32_t bx,int32_t by,int32_t px,int32_t py) {
    return ((int64_t)px-ax)*((int64_t)by-ay)-((int64_t)py-ay)*((int64_t)bx-ax);
}

bool psre_m032_fill_triangle(int32_t x0,int32_t y0,int32_t x1,int32_t y1,int32_t x2,int32_t y2,psre_m032_color_t c) {
    if (!ready() || !coord_ok(x0)||!coord_ok(y0)||!coord_ok(x1)||!coord_ok(y1)||!coord_ok(x2)||!coord_ok(y2)) return false;
    const psre_m031_framebuffer_t *fb=psre_m031_get();
    int32_t minx=x0<x1?x0:x1; if(x2<minx)minx=x2;
    int32_t maxx=x0>x1?x0:x1; if(x2>maxx)maxx=x2;
    int32_t miny=y0<y1?y0:y1; if(y2<miny)miny=y2;
    int32_t maxy=y0>y1?y0:y1; if(y2>maxy)maxy=y2;
    if (minx < 0) minx = 0;
    if (miny < 0) miny = 0;
    if(maxx>=(int32_t)fb->width)maxx=(int32_t)fb->width-1;
    if(maxy>=(int32_t)fb->height)maxy=(int32_t)fb->height-1;
    if(minx>maxx||miny>maxy)return false;
    int64_t area=edge(x0,y0,x1,y1,x2,y2); if(!area)return false;
    bool positive=area>0, drew=false;
    for(int32_t y=miny;y<=maxy;++y) for(int32_t x=minx;x<=maxx;++x) {
        int64_t e0=edge(x0,y0,x1,y1,x,y),e1=edge(x1,y1,x2,y2,x,y),e2=edge(x2,y2,x0,y0,x,y);
        if(positive?(e0>=0&&e1>=0&&e2>=0):(e0<=0&&e1<=0&&e2<=0)) {
            (void)psre_m031_put_pixel((uint64_t)x,(uint64_t)y,c.red,c.green,c.blue); drew=true;
        }
    }
    if (drew) ++g_operations;
    return drew;
}

bool psre_m032_draw_bitmap(int32_t x,int32_t y,const psre_m032_bitmap_t *bm) {
    if(!ready()||!bm||!bm->pixels||!bm->width||!bm->height||
       (bm->format!=PSRE_M032_BITMAP_RGB24&&bm->format!=PSRE_M032_BITMAP_BGR24)||
       bm->width>16384u||bm->height>16384u||bm->stride<bm->width*3u) return false;
    psre_m032_rect_t r={x,y,(int32_t)bm->width,(int32_t)bm->height};
    if(!psre_m032_clip_rect(&r))return false;
    uint32_t sx=(uint32_t)(r.x-x),sy=(uint32_t)(r.y-y);
    for(int32_t py=0;py<r.height;++py) for(int32_t px=0;px<r.width;++px) {
        const uint8_t *p=bm->pixels+(uint64_t)(sy+(uint32_t)py)*bm->stride+(uint64_t)(sx+(uint32_t)px)*3u;
        uint8_t red=p[bm->format==PSRE_M032_BITMAP_RGB24?0:2];
        uint8_t blue=p[bm->format==PSRE_M032_BITMAP_RGB24?2:0];
        (void)psre_m031_put_pixel((uint64_t)(r.x+px),(uint64_t)(r.y+py),red,p[1],blue);
    }
    ++g_operations;return true;
}

bool psre_m032_draw_demo(void) {
    if (!ready()) return false;
    const psre_m031_framebuffer_t *fb = psre_m031_get();
    if (fb->width < 16u || fb->height < 16u) return false;
    int32_t w = (int32_t)fb->width, h = (int32_t)fb->height;
    int32_t margin = w / 32; if (margin < 1) margin = 1;
    int32_t radius = (w < h ? w : h) / 10; if (radius < 2) radius = 2;
    int32_t cy = h / 3;
    bool ok = psre_m032_clear(C_BLACK);
    ok = psre_m032_fill_rect(margin, margin, w / 6, h / 5, C_BLUE) && ok;
    ok = psre_m032_draw_rect(margin, margin, w / 6, h / 5, C_WHITE) && ok;
    ok = psre_m032_draw_line(margin, h / 2, w - margin - 1, h / 2, C_RED) && ok;
    ok = psre_m032_draw_circle(w / 3, cy, radius, C_GREEN) && ok;
    ok = psre_m032_fill_circle((w * 2) / 3, cy, radius, C_YELLOW) && ok;
    ok = psre_m032_draw_triangle(w / 8, h - margin - 1, w / 4, h / 2,
                                 w * 3 / 8, h - margin - 1, C_WHITE) && ok;
    ok = psre_m032_fill_triangle(w * 5 / 8, h - margin - 1, w * 3 / 4, h / 2,
                                 w * 7 / 8, h - margin - 1, C_RED) && ok;
    return ok;
}

void psre_m032_report(void) {
    const psre_m031_framebuffer_t *fb=psre_m031_get();
    serial_write("[M032] Graphics Primitive Engine: ");
    serial_write(ready()?"ready":"not initialized");
    serial_write(" | framebuffer: ");
    if(fb->available) { serial_write("available "); }
    else serial_write("unavailable ");
    serial_write("| operations: ");
    char b[21];unsigned n=0;uint64_t v=g_operations;
    do{b[n++]=(char)('0'+v%10u);v/=10u;}while(v&&n<sizeof(b));
    while (n) serial_write_char(b[--n]);
    serial_write("\n");
}
