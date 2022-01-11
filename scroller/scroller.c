#include "demo.h"

int main() {
    struct bitmap font;
    const char text[] = "YEAH !!  QUE PASA TIOS !!  ITS GREAT TO BE BACK HERE IN BARCELONA TONIGHT !!  1 2 3 4  !!  \0";
    int i, x, y;
    int xoffset, yoffset;
    int ch, col;

    font = read_bmp("font.bmp");

    set_mode(0x13);
    set_palette(font.palette);

/*
    draw_bmp(&image, 0, 0);
    wait(18);

    for (i = 0; i < 320; i++) {
        for (y = 51; y < 100; y++) {
            memcpy(&vga[y*320+81], &vga[y*320+82], 159);
            vga[y*320+239] = 180;
        }
        wait_for_retrace();
        wait_for_retrace();
    }

    wait(18);
*/
    ch = 0;
    col = 0;
    while (!kbhit()) {
        xoffset = (text[ch]-32)*8+col;

        for (y = 96; y < 104; y++) {
            memcpy(&vga[y*320], &vga[y*320+1], 320);
            vga[y*320+319] = font.data[(y-96)*font.width+xoffset];

            if (col == 7) {
                memcpy(&vga[y*320], &vga[y*320+1], 320);
                vga[y*320+319] = 0;
            }
        }

        col++;
        if (col > 7) {
            ch++;
            col = 0;
        }
        if (ch == strlen(text))
            ch = 0;

        wait_for_retrace();
    }

    set_mode(0x03);

    free(font.palette);
    free(font.data);

    return 0;
}