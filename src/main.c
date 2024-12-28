#include <stdio.h>
#include <sys/ioctl.h>
#include <signal.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include "r2_termui.h"

//                        0       8      16      24      32      40      48      56      64
//                        01234567812345678123456781234567812345678123456781234567812345678
//                        |-------|-------|-------|-------|-------|-------|-------|-------|
const char* ASCIICHARS = " `,_-~+:;!litfjrxnuvczmwqpdbkhao*^I?][{}17()|/XYUJCLQ0OZ$#MW&8%B@";

int main(int argv, const char** argc)
{
    if (argv <= 1)
    {
        printf("Need an image\n");
        return 1;
    }

    // Get the terminal windows size
    struct winsize sz;
    ioctl(0, TIOCGWINSZ, &sz);
    // int limit = sz.ws_row * sz.ws_col;
    // printf("%dx%d\n", sz.ws_row, sz.ws_col);

    // load and display the iamge (as best we can)
    const char * filename = argc[1];
    int w, h, n;
    unsigned char *image_data = stbi_load(filename, &w, &h, &n, STBIR_RGB);
    // printf("%dx%d\n", w, h);
    if (image_data != NULL)
    {
        float resize_percent = (float)(sz.ws_row - 2) / (float)w;
        // float resize_percent = (float)sz.ws_col / h;

        int output_w = (int)w * resize_percent;
        int output_h = (int)h * resize_percent;
        // printf("%f\n", resize_percent);
        // printf("%dx%d %d\n", output_w, output_h, n);
        unsigned char *output_pixels = calloc(sizeof(unsigned char), output_w * output_h * n);

        stbir_resize_uint8_linear(
            image_data,  w,  h,  n*w,
            output_pixels, output_w, output_h, n*output_w,
            STBIR_RGB);

        // A 'pixel' in the terminal is not exactly a square. It could be,
        // for example: one character = 9x18px. That means, most often, the
        // pixels will be taller than they are wide.
        for (int y = 0; y < output_h; y++)
        {
            for (int x = 0; x < output_w; x++)
            {
                int pix = (y * output_w + x) * n;
                // https://en.wikipedia.org/wiki/Relative_luminance
                int R = output_pixels[pix + 0];
                int G = output_pixels[pix + 1];
                int B = output_pixels[pix + 2];
                // int b = (0.2126*R + 0.7152*G + 0.0722*B) / 64;
                int b = (R + G + B) / 64;

                // if (R > G && R > B)
                //     printf(ESC_SET_ATTRIBUTE_MODE_1, 41);
                // else if (G > R && G > B)
                //     printf(ESC_SET_ATTRIBUTE_MODE_1, 42);
                // else if (B > R && B > G)
                //     printf(ESC_SET_ATTRIBUTE_MODE_1, 44);
                // else
                //     printf(ESC_SET_ATTRIBUTE_MODE_1, 40);

                printf("%c%c", ASCIICHARS[b], ASCIICHARS[b]);

            }
            printf("\n");
        }
        free(output_pixels);
    }

    stbi_image_free(image_data);
    return 0;
}
