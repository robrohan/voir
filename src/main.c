#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>    /* for getopt */
#include <signal.h>
#include <locale.h>
#include <assert.h>

#ifndef _WIN32
#include <sys/ioctl.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#define R2_STRINGS_IMPLEMENTATION
#include "r2_strings.h"

#include "r2_termui.h"


int color_to_ascii_index(unsigned char R, unsigned char G, unsigned char B, int range)
{
    // int b = (0.2126*R + 0.7152*G + 0.0722*B) / 64;
    int r = range-1;
    float c = (R + G + B);
    float m = c / 765.0f;
    int b = r * m;
    assert(b >= 0 && b <= r);
    return b;
}

void set_background_color(unsigned char R, unsigned char G, unsigned char B)
{
    // setting RGB only works on limited terminals
    printf(ESC_SET_ATTRIBUTE_MODE_BACKGROUND_RGB, R, G, B);
}

void show_usage(const char *app)
{
    printf("%s: basic image viewer for the terminal \n", app);
    printf("Basic usage:\n");
    printf("\t %s -i <image>\n", app);
    printf("Options:\n");
    printf("\t -a - only use ascii chars (default utf8)\n");
    printf("\t -d - use dark mode\n");
    printf("\t -c - use colors\n");
    printf("\t -h - help screen; this\n");
    printf("\n");
}

int main(int argc, const char** argv)
{
    // Assume en_US is on the system C.UTF-8 doesn't seem to work
    // on MacOS
    setlocale(LC_ALL, "en_US.UTF-8");

    const char * filename = NULL;
    bool use_color = false;
    bool ascii_only = false;
    bool dark_mode = false;
    int c;
    while ((c = getopt(argc, (char**)argv, "chadi:")) != -1)
    {
        switch(c)
        {
            case 'i':
                filename = optarg;
                break;
            case 'c':
                // add colors
                use_color = true;
                break;
            case 'a':
                // use ascii only (no utf8)
                ascii_only = true;
                break;
            case 'd':
                // light to dark vs dark to light
                dark_mode = true;
                break;
            case 'h':
                // help
                show_usage(argv[0]);
                return 0;
            case '?':
                break;
            default:
                show_usage(argv[0]);
        }
    }
    if (filename == NULL)
    {
        show_usage(argv[0]);
        return 1;
    }

    s8 ASCIICHARS;
    if (ascii_only)
    {
        //              0       8      16      24      32      40      48      56      64
        //              01234567812345678123456781234567812345678123456781234567812345678
        //              |-------|-------|-------|-------|-------|-------|-------|-------|
        ASCIICHARS = S("`,_-~+:;!litfjrxnuvczmwqpdbkhao*^I?][{}17()|/XYUJCLQ0OZ$#MW&8%B@");
    }
    else
    {
        //                  0       8
        //                  |-------|
        if(dark_mode)
            ASCIICHARS = S("▁░▒▓█");
        else
            ASCIICHARS = S("█▓▒░▁");
    }

    float resize_percent = 1;

#ifndef _WIN32
    // Get the terminal windows size
    struct winsize sz;
    ioctl(0, TIOCGWINSZ, &sz);
    // int limit = sz.ws_row * sz.ws_col;
#endif

    // load and display the iamge (as best we can)
    int w, h, n;
    unsigned char *image_data = stbi_load(filename, &w, &h, &n, STBIR_RGB);
    if (image_data != NULL)
    {

#ifndef _WIN32
        if (w > sz.ws_row)
            resize_percent = (float)(sz.ws_row - 2) / (float)w;
#endif

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
                int b = color_to_ascii_index(R, G, B, ASCIICHARS.len);
                if(use_color) set_background_color(R, G, B);
                printf("%lc%lc", (int)ASCIICHARS.rune[b], (int)ASCIICHARS.rune[b]);
                if(use_color) set_background_color(0, 0, 0);
            }
            printf("\n");
        }
        free(output_pixels);
    }

    free_S(ASCIICHARS);
    stbi_image_free(image_data);
    return 0;
}
