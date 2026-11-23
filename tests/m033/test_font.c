#include <psre/m033/font.h>
#include <psre/m031/framebuffer.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint8_t pixels[64*64*4];
static psre_m031_framebuffer_t fb={.available=true,.address=pixels,.byte_size=sizeof(pixels),
 .width=64,.height=64,.pitch=64*4,.bits_per_pixel=32,.bytes_per_pixel=4,
 .red_mask_size=8,.red_mask_shift=16,.green_mask_size=8,.green_mask_shift=8,
 .blue_mask_size=8,.blue_mask_shift=0,.pixel_model=PSRE_M031_PIXEL_RGB};
const psre_m031_framebuffer_t *psre_m031_get(void){return &fb;}
bool psre_m031_put_pixel(uint64_t x,uint64_t y,uint8_t r,uint8_t g,uint8_t b){
 if(x>=fb.width||y>=fb.height) return false;
 uint64_t o=y*fb.pitch+x*4;
 pixels[o]=(uint8_t)b;pixels[o+1]=(uint8_t)g;pixels[o+2]=(uint8_t)r;pixels[o+3]=0;return true;
}
bool psre_m031_fill_rect(uint64_t x,uint64_t y,uint64_t w,uint64_t h,uint8_t r,uint8_t g,uint8_t b){
 bool ok=false;for(uint64_t yy=y;yy<y+h&&yy<fb.height;yy++)for(uint64_t xx=x;xx<x+w&&xx<fb.width;xx++)ok=psre_m031_put_pixel(xx,yy,r,g,b)||ok;return ok;
}
bool psre_m032_initialize(void){return fb.available;}
bool psre_m032_draw_pixel(int32_t x,int32_t y,psre_m032_color_t c){if(x<0||y<0)return false;return psre_m031_put_pixel((uint32_t)x,(uint32_t)y,c.red,c.green,c.blue);}
bool psre_m032_clear(psre_m032_color_t c){return psre_m031_fill_rect(0,0,fb.width,fb.height,c.red,c.green,c.blue);}
bool psre_m032_fill_rect(int32_t x,int32_t y,int32_t w,int32_t h,psre_m032_color_t c){if(x<0||y<0||w<=0||h<=0)return false;return psre_m031_fill_rect((uint32_t)x,(uint32_t)y,(uint32_t)w,(uint32_t)h,c.red,c.green,c.blue);}
bool psre_m032_draw_line(int32_t x0,int32_t y0,int32_t x1,int32_t y1,psre_m032_color_t c){(void)x1;(void)y1;return psre_m032_draw_pixel(x0,y0,c);}
void serial_write(const char *s){(void)s;}
void serial_write_char(char c){(void)c;}
static unsigned lit_pixels(void){unsigned n=0;for(unsigned i=0;i<sizeof(pixels);i+=4)if(pixels[i]||pixels[i+1]||pixels[i+2])++n;return n;}
int main(void){
 assert(psre_m033_initialize());
 psre_m032_color_t white={255,255,255},black={0,0,0};
 assert(psre_m033_draw_char(2,2,'A',PSRE_M033_FONT_8X8,white,black,false));
 assert(lit_pixels()>0); unsigned first=lit_pixels();
 assert(psre_m033_draw_char(12,2,'B',PSRE_M033_FONT_8X16,white,black,false));
 assert(lit_pixels()>first);
 psre_m033_text_style_t st={.x=0,.y=24,.width=64,.height=32,.spacing=1,.font=PSRE_M033_FONT_8X8,
 .foreground=white,.background=black,.opaque_background=true};
 assert(psre_m033_draw_string("PSRE\nM033",&st));
 psre_m033_text_size_t z=psre_m033_measure("AB\nCD",PSRE_M033_FONT_8X8,1);
 assert(z.width==17 && z.height==16);
 assert(psre_m033_draw_char(-4,50,'Z',PSRE_M033_FONT_8X16,white,black,false));
 memset(pixels,0,sizeof(pixels));
 st.x=10; st.y=10; st.width=8; st.height=8; st.font=PSRE_M033_FONT_8X16;
 assert(psre_m033_draw_string("A",&st));
 for(unsigned yy=0;yy<64;yy++) for(unsigned xx=0;xx<64;xx++) {
   bool lit=pixels[(yy*64+xx)*4]||pixels[(yy*64+xx)*4+1]||pixels[(yy*64+xx)*4+2];
   if(lit) assert(xx>=10 && xx<18 && yy>=10 && yy<18);
 }
 /* ---- UI large mode: taller glyphs, identical horizontal advance ---- */
 assert(!psre_m033_ui_large() && psre_m033_ui_line_height()==8);
 memset(pixels,0,sizeof(pixels));
 psre_m033_text_style_t ls={.x=10,.y=10,.width=64,.height=12,.spacing=1,.font=PSRE_M033_FONT_8X8,
   .foreground=white,.background=black,.opaque_background=false};
 assert(psre_m033_draw_string("A",&ls));
 int cmin=99,cmax=-1; for(int yy=0;yy<64;yy++)for(int xx=0;xx<64;xx++)if(pixels[(yy*64+xx)*4]){if(yy<cmin)cmin=yy;if(yy>cmax)cmax=yy;}
 int classic_h=cmax-cmin+1;
 psre_m033_set_ui_large(true);
 assert(psre_m033_ui_large() && psre_m033_ui_line_height()==14);
 z=psre_m033_measure("AB\nCD",PSRE_M033_FONT_8X8,1);
 assert(z.width==17 && z.height==28);                 /* same width, taller lines */
 memset(pixels,0,sizeof(pixels));
 assert(psre_m033_draw_string("A",&ls));
 cmin=99;cmax=-1; for(int yy=0;yy<64;yy++)for(int xx=0;xx<64;xx++)if(pixels[(yy*64+xx)*4]){if(yy<cmin)cmin=yy;if(yy>cmax)cmax=yy;}
 assert(cmax-cmin+1>classic_h+2);                     /* visibly larger capital letter */
 /* a descender is not cut by a caller box sized for 8 px text (height 12) */
 memset(pixels,0,sizeof(pixels));
 assert(psre_m033_draw_string("g",&ls));
 cmax=-1; for(int yy=0;yy<64;yy++)for(int xx=0;xx<64;xx++)if(pixels[(yy*64+xx)*4]&&yy>cmax)cmax=yy;
 assert(cmax==10+12);
 /* multi-line text steps by the larger pitch (14 px) */
 memset(pixels,0,sizeof(pixels));
 ls.height=40; assert(psre_m033_draw_string("A\nA",&ls));
 int r1=99,r2=99;
 for(int yy=0;yy<64;yy++)for(int xx=0;xx<64;xx++)if(pixels[(yy*64+xx)*4]){if(yy<r1)r1=yy;}
 for(int yy=r1+9;yy<64;yy++)for(int xx=0;xx<64;xx++)if(pixels[(yy*64+xx)*4]&&yy<r2)r2=yy;
 assert(r2==r1+14);
 /* the explicit 8x16 font is unaffected by the UI mode and mode can be switched back */
 psre_m033_set_ui_large(false);
 assert(!psre_m033_ui_large() && psre_m033_ui_line_height()==8);
 z=psre_m033_measure("AB\nCD",PSRE_M033_FONT_8X8,1); assert(z.width==17 && z.height==16);
 puts("M033 font renderer tests: PASS");return 0;
}
