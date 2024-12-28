#include <stdio.h>
#include <signal.h>

#ifndef _WIN32
#include <sys/ioctl.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

// #include "r2_termui.h"

//                        0       8      16      24      32      40      48      56      64
//                        01234567812345678123456781234567812345678123456781234567812345678
//                        |-------|-------|-------|-------|-------|-------|-------|-------|
const char* ASCIICHARS = " `,_-~+:;!litfjrxnuvczmwqpdbkhao*^I?][{}17()|/XYUJCLQ0OZ$#MW&8%B@";

unsigned char color_to_ascii_index(unsigned char R, unsigned char G, unsigned char B)
{
    // int b = (0.2126*R + 0.7152*G + 0.0722*B) / 64;
    int b = (R + G + B) / 64;
    return (unsigned char) b;
}

void set_background_color(unsigned char R, unsigned char G, unsigned char B)
{
    // if (R > G && R > B)
    //     printf(ESC_SET_ATTRIBUTE_MODE_1, 41);
    // else if (G > R && G > B)
    //     printf(ESC_SET_ATTRIBUTE_MODE_1, 42);
    // else if (B > R && B > G)
    //     printf(ESC_SET_ATTRIBUTE_MODE_1, 44);
    // else
    //     printf(ESC_SET_ATTRIBUTE_MODE_1, 40);
    // printf(ESC_SET_ATTRIBUTE_MODE_RGB, R, G, B);
}

int main(int argv, const char** argc)
{
    if (argv <= 1)
    {
        printf("Need an image\n");
        printf("%s <image>\n", argc[0]);
        return 1;
    }

    // Get the terminal windows size
    struct winsize sz;
    ioctl(0, TIOCGWINSZ, &sz);
    // int limit = sz.ws_row * sz.ws_col;

    // load and display the iamge (as best we can)
    const char * filename = argc[1];
    int w, h, n;
    unsigned char *image_data = stbi_load(filename, &w, &h, &n, STBIR_RGB);
    if (image_data != NULL)
    {
        float resize_percent = 1;
        if (w > sz.ws_row)
            resize_percent = (float)(sz.ws_row - 2) / (float)w;

        int output_w = (int)w * resize_percent;
        int output_h = (int)h * resize_percent;
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
                unsigned char R = output_pixels[pix + 0];
                unsigned char G = output_pixels[pix + 1];
                unsigned char B = output_pixels[pix + 2];
                unsigned char b = color_to_ascii_index(R, G, B);
                // set_background_color();
                printf("%c%c", ASCIICHARS[b], ASCIICHARS[b]);
            }
            // reset the color
            // printf(ESC_SET_ATTRIBUTE_MODE_1, 40);
            printf("\n");
        }
        free(output_pixels);
    }

    stbi_image_free(image_data);
    return 0;
}
