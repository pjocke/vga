#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <math.h>
#include <mem.h>

extern unsigned char *vga = (unsigned char *)0xA0000000L;
extern unsigned short *clock = (unsigned short *)0x0000046C;

extern struct rgb_color {
    unsigned char r, g, b;
};

extern 
struct bitmap {
    unsigned long width;
    unsigned long height;
    unsigned char *palette;
    unsigned char *data;
};

extern void set_mode(int mode);
extern void set_palette(unsigned char *palette);
extern struct rgb_color hsv_to_rgb(float h, float s, float v);
extern void wait_for_retrace(void);
extern void wait(int ticks);
extern struct bitmap read_bmp(char *filename);

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

struct bitmap read_bmp(char *filename) {
    struct bitmap bmp;
    FILE *fp;
    unsigned long compression, colors;
    unsigned short depth;
    int i, j;
    long x, y;

    if ((fp = fopen(filename, "rb")) == NULL) {
        printf("Error opening file %s.\n", filename);
        exit(1);
    }

    /* Type - 2 bytes */
    if (fgetc(fp) != 'B' || fgetc(fp) != 'M') {
        printf("%s is not a bitmap file.\n", filename);
        exit(1);
    }

    /* File size, reserved, reserved, offset, header size - 4 + 2 + 2 + 4 = 16 bytes */
    fseek(fp, 16, SEEK_CUR);

    /* Widht, height - 4 + 4 bytes */
    fread(&bmp.width, 4, 1, fp);
    fread(&bmp.height, 4, 1, fp);

    /* Color planes - 2 bytes */
    fseek(fp, 2, SEEK_CUR);

    /* Bits per pixel - 2 bytes */
    fread(&depth, 2, 1, fp);

    /* Compression - 4 bytes */
    fread(&compression, 4, 1, fp);
    if ((int) compression != 0) {
        printf("%s is compressed.\n");
        exit(1);
    }

    /* Image size, horizontal resolution, vertical resolution - 4 + 4 + 4 = 12 bytes */
    fseek(fp, 12, SEEK_CUR);

    /* Number of colors, 0 == 2^depth - 4 bytes */
    fread(&colors, 4, 1, fp);
    if ((int) colors == 0)
        colors = (unsigned long) pow(2, (int) depth);

    /* Important colors - 4 bytes */
    fseek(fp, 4, SEEK_CUR);

    /* Palette, 6 bits per color - 2^depth * 4 bytes */
    if ((bmp.palette = malloc((size_t) (int) colors * 3)) == NULL) {
        printf("Unable to allocate memory for palette.\n");
        fclose(fp);
        exit(1);
    }

    for (i = 0; i < (int) colors; i++) {
        j = i*3;
        /* Blue */
        bmp.palette[j+2] = fgetc(fp) >> 2;
        /* Green */
        bmp.palette[j+1] = fgetc(fp) >> 2;
        /* Red */
        bmp.palette[j] = fgetc(fp) >> 2;
        /* Alpha */
        fseek(fp, 1, SEEK_CUR);
    }

    /* Bitmap data, starting from bottom left corner */
    if ((bmp.data = malloc((size_t) (int) bmp.width * (int) bmp.height)) == NULL) {
        printf("Unable to allocate memory for bitmap data.\n");
        free(bmp.palette);
        fclose(fp);
        exit(1);
    }

    for (y = (long) bmp.width * ((long) bmp.height-1); y >= 0; y -= (long) bmp.width) {
        for (x = 0; x < (long) bmp.width; x++) {
            bmp.data[y+x] = (unsigned char) fgetc(fp);
        }
        /* Pad to nearest multiple of 4 */
        fseek(fp, (int) (ceil(((double) depth * (double) bmp.width)/32)*4)-(int) bmp.width, SEEK_CUR);
    }

    fclose(fp);

    return bmp;
}

void draw_bmp(struct bitmap *bmp, int x, int y) {
    int i;
    unsigned short offset_s = (y<<6)+(y<<8)+x;
    unsigned short offset_b = 0;

    for (i = 0; i < bmp->height; i++) {
        memcpy(&vga[offset_s], &bmp->data[offset_b], bmp->width);

        offset_s += 320;
        offset_b += bmp->width;
    }
}