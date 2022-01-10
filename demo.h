#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <mem.h>

extern unsigned char *vga = (unsigned char *)0xA0000000L;
extern unsigned short *clock = (unsigned short *)0x0000046C;

extern struct rgb_color {
    unsigned char r, g, b;
};

extern void set_mode(int mode);
extern void set_palette(unsigned char *palette);
extern struct rgb_color hsv_to_rgb(float h, float s, float v);
extern void wait_for_retrace(void);
extern void wait(int ticks);

extern void set_mode(int mode) {
  union REGS regs;

  regs.h.ah = 0x00;
  regs.h.al = mode;
  int86(0x10, &regs, &regs);
}

extern void set_palette(unsigned char *palette) {
    register int i;

    outp(0x03c8, 0);

    for (i = 0; i < 256*3; i++) {
        outp(0x03c9, palette[i]);
    }
}

extern struct rgb_color hsv_to_rgb(float h, float s, float v) {
    float v2 = v/100.0;
    float c = v2*(s/100.0);
    float x = c*(1.0-fabs(fmod(h/60.0, 2)-1.0));
    float m = v2-c;
    float r, g, b;
    struct rgb_color rgb;

    if (h >= 0 && h < 60) {
        r = c, g = x, b = 0;
    } else if (h >= 60 && h < 120) {
        r = x, g = c, b = 0;
    } else if (h >= 120 && h < 180) {
        r = 0, g = c, b = x;
    } else if (h >= 180 && h < 240) {
        r = 0, g = x, b = c;
    } else if (h >= 240 && h < 300) {
        r = x, g = 0, b = c;
    } else {
        r = c, g = 0, b = x;
    }

    rgb.r = (r+m)*255.0;
    rgb.g = (g+m)*255.0;
    rgb.b = (b+m)*255.0;

    return rgb;
}

extern void wait_for_retrace(void)
{
    while  ((inp(0x03da) & 0x08)) {};
    while (!(inp(0x03da) & 0x08)) {};
}

extern void wait(int ticks)
{
  unsigned short start = *clock;

  while (*clock-start<ticks) {}
}
