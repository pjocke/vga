#include "demo.h"

int main() {
    struct bitmap font;
    struct bitmap image;
    const char text[] = "IT S PEANUT BUTTER JELLY TIME!  2022 WILL MARK THE 20 TH ANNIVERSARY OF THIS GLORIOUS DANCING BANANA BUSTING ITS MOVES TO THE AMAZING TUNE  PEANUT BUTTER JELLY TIME  BY THE BUCKWHEAT BOYZ   NEVER FORGET!                   \0";
    unsigned int offset, y, count;
    unsigned char *buff;

    if ((buff = malloc(320*200)) == NULL) {
        printf("Unable to allocate memory for double buffer.\n");
        exit(1);
    }
    memset(buff, 0, 320*200);

    font = read_bmp("font.bmp");
    free(font.palette);

    image = read_bmp("banana.bmp");

    set_mode(0x13);

    set_palette(image.palette);

    count = 0;
    while (!kbhit()) {
        /* Image */
        if (count%6 == 0) {
            offset = ((count/6)%8)*80;
            for (y = 0; y < 80; y++) {
                memcpy(&buff[(y+52)*320+120], &image.data[y*640+offset], 80);
            }
        }

        /* Scroller */
        offset = (text[(count/8)%strlen(text)]-32)*8+(count%8);
        for (y = 140; y < 148; y++) {
            memcpy(&buff[y*320+80], &buff[y*320+81], 160);
            buff[y*320+240] = font.data[(y-140)*font.width+offset];

            if (count%8 == 7) {
                memcpy(&buff[y*320+80], &buff[y*320+81], 160);
                buff[y*320+240] = 0;
            }
        }

        /* Draw */
        wait_for_retrace();
        memcpy(vga, buff, 320*200);

        /* Count stuff */
        count++;
    }

    set_mode(0x03);

    free(font.data);
    free(image.palette);
    free(image.data);

    free(buff);

    return 0;
}