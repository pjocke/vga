#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dos.h>
#include <conio.h>

#define SIDE 129

/* Globals */
unsigned char *vga = (unsigned char *)0xA0000000L;
unsigned short *clock = (unsigned short *)0x0000046C;

struct rgb_color {
    unsigned char r, g, b;
};

/* Declarations */
void diamondsquare(int **a, register int size);
void set_mode(int mode);
void set_palette(unsigned char *palette);
void plot(int **a);
struct rgb_color hsv_to_rgb(float h, float s, float v);
void wait_for_retrace(void);

/* Main motherfucker */
int main() {
    int *a[SIDE];
    int **a2;
    int tmp;
    register int i, x, y, j, c;
    struct rgb_color color;
    unsigned char rainbow[1080];
    unsigned char palette[768];

    /* Initialize two-dimensional array */
    for (i = 0; i < SIDE; i++)
        a[i] = (int*)malloc(SIDE * sizeof(int));

    /* Set all values to 0 */
    for (y = 0; y < SIDE; y++)
        for (x = 0; x < SIDE; x++)
            a[y][x] = 0;

    /* Cut the deck */
    srand(*clock);

    /* Save some arithmetic for later */
    tmp = SIDE-1;

    /* Initialize corners */
    a[0][0]        = rand()%SIDE; /* NW */
    a[tmp][tmp]    = rand()%SIDE; /* SE */
    a[0][tmp]      = rand()%SIDE; /* NE */
    a[tmp][0]      = rand()%SIDE; /* SW */

    /* Square those diamonds, baby */
    diamondsquare(a, tmp);

    /*
       I can't think of anything more rewarding than being able to express yourself to others through painting.
         - Bob Ross
           - Michael Scott
    */
    for (i = 0; i < 360; i++) {
        color = hsv_to_rgb((float) i+1, 100.0, 100.0);
        j = i*3;
        rainbow[j] = color.r/4;
        rainbow[j+1] = color.g/4;
        rainbow[j+2] = color.b/4;
    }

    palette[0] = 0, palette[1] = 0, palette[2] = 0;
    for (i = 3; i < 768; i++) {
        palette[i] = rainbow[i];
    }

    set_mode(0x13);
    set_palette(palette);
    plot(a);

    /* I want to ride my palette cycle */
    i = 0;
    while (!kbhit()) {
        for (j = 3; j < 768; j+=3) {
            palette[j] = rainbow[(i+j)%1080];
            palette[j+1] = rainbow[(i+j+1)%1080];
            palette[j+2] = rainbow[(i+j+2)%1080];
        }

        wait_for_retrace();
        wait_for_retrace();

        set_palette(palette);

        i += 3;
        if (i >= 1080) {
            i = 0;
        }
    }

    set_mode(0x03);

    /* Free allocated memory */
    for (i = 0; i < SIDE; i++)
        free(a[i]);

    /* It's wabbit season */
    printf("Fuddings to Elmers.\n");

    return 0;
}

/* Definitions*/

/* Do the dance */
void diamondsquare(int **a, register int size) {
    register int half = size/2;
    register int x, y;

    /* Is this really necessary? */
    srand(*clock);

    /* Diamond step, find all squares */
    for (y = 0; y < SIDE-size; y += size) {
        for (x = 0; x < SIDE-size; x += size) {
            a[y+half][x+half] = (a[y][x] + a[y+size][x+size] + a[y][x+size] + a[y+size][x])/4 + rand()%size;
        }
    }

    /* Square step, find all diamonds */
    for (y = half; y < SIDE; y += size) {
        for (x = half; x < SIDE; x += size) {
            /* N */
            if (a[y-half][x] == 0) {
                if (y-size > 0) {
                    a[y-half][x] = (a[y-size][x] + a[y][x] + a[y-half][x+half] + a[y-half][x-half])/4 + rand()%size;
                } else {
                    a[y-half][x] = (a[y][x] + a[y-half][x+half] + a[y-half][x-half])/3 + rand()%size;
                }
            }

            /* S */
            if (a[y+half][x] == 0) {
                if (y+size < SIDE) {
                    a[y+half][x] = (a[y][x] + a[y+size][x] + a[y+half][x+half] + a[y+half][x-half])/4 + rand()%size;
                } else {
                    a[y+half][x] = (a[y][x] + a[y+half][x+half] + a[y+half][x-half])/3 + rand()%size;
                }
            }

            /* E */
            if (a[y][x+half] == 0) {
                if (x+size < SIDE) {
                    a[y][x+half] = (a[y-half][x+half] + a[y+half][x+half] + a[y][x+size] + a[y][x])/4 + rand()%size;
                } else {
                    a[y][x+half] = (a[y-half][x+half] + a[y+half][x+half] + a[y][x])/3 + rand()%size;
                }
            }

            /* W */
            if (a[y][x-half] == 0) {
                if (x-size > 0) {
                    a[y][x-half] = (a[y-half][x-half] + a[y+half][x-half] + a[y][x] + a[y][x-size])/4 + rand()%size;
                } else {
                    a[y][x-half] = (a[y-half][x-half] + a[y+half][x-half] + a[y][x])/3 + rand()%size;
                }
            }
        }
    }

    if (size/2 >= 2)
        diamondsquare(a, size/2);
}

/* Get in the mode */
void set_mode(int mode) {
  union REGS regs;

  regs.h.ah = 0x00;
  regs.h.al = mode;
  int86(0x10, &regs, &regs);
}

/* Happy little accidents */
void set_palette(unsigned char *palette) {
    register int i;

    outp(0x03c8, 0);

    for (i = 0; i < 256*3; i++) {
        outp(0x03c9, palette[i]);
    }
}

/* Make a little noice */
void plot(int **a) {
    register int x, y, y2;

    for (y = 0; y < SIDE; y++)
        for (x = 0; x < SIDE; x++) {
            y2 = y+(200-SIDE)/2;
            vga[(y2<<6)+(y2<<8)+x+(320-SIDE)/2] = a[y][x];
        }
}

/* Technicolor Boggle */
struct rgb_color hsv_to_rgb(float h, float s, float v) {
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

void wait_for_retrace(void)
{
    while  ((inp(0x03da) & 0x08)) {};
    while (!(inp(0x03da) & 0x08)) {};
}
