/*************************************************
 * https://en.wikipedia.org/wiki/BMP_file_format *
 *************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct bitmap {
    unsigned long width;
    unsigned long height;
    unsigned char *palette;
    unsigned char *data;
};

struct bitmap read_bmp(char *filename) {
    struct bitmap bmp;
    FILE *fp;
    unsigned long compression, colors;
    unsigned short depth;
    int i, j, x, y;

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

    /* Bitmap data, starting from bottom left corner, ignoring padding */
    if ((bmp.data = malloc((size_t) (int) bmp.width * (int) bmp.height)) == NULL) {
        printf("Unable to allocate memory for bitmap data.\n");
        free(bmp.palette);
        fclose(fp);
        exit(1);
    }

    for (y = (int) bmp.width * ((int) bmp.height-1); y >= 0; y -= (int) bmp.width) {
        for (x = 0; x < (int) bmp.width; x++) {
            bmp.data[y+x] = fgetc(fp);
        }
    }

    fclose(fp);

    return bmp;
}
