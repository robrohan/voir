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

typedef struct ansi_color {
    int code;
    int rgb[3];
} ansi_color;

#define NUM_COLORS 16
ansi_color ANSI_COLORS[NUM_COLORS] = {
    { .code=40, .rgb={0,0,0} },
    { .code=41, .rgb={170,0,0} },
    { .code=42, .rgb={0,170,0} },
    { .code=43, .rgb={170,85,0} },
    { .code=44, .rgb={0,0,170} },
    { .code=45, .rgb={170,0,170} },
    { .code=46, .rgb={0,170,170} },
    { .code=47, .rgb={170,170,170} },
    { .code=100, .rgb={85,85,85} },
    { .code=101, .rgb={255,85,85} },
    { .code=102, .rgb={85,255,85} },
    { .code=103, .rgb={255,255,85} },
    { .code=104, .rgb={85,85,255} },
    { .code=105, .rgb={255,85,255} },
    { .code=106, .rgb={85,255,255} },
    { .code=107, .rgb={255,255,255} }
};

int color_serach(int R, int G, int B)
{
    float smallest = 1000.0;
    int smallest_idx = NUM_COLORS;
    for (int c = 0; c < NUM_COLORS; c++)
    {
        float n1 = (ANSI_COLORS[c].rgb[0] - R);
        float n2 = (ANSI_COLORS[c].rgb[1] - G);
        float n3 = (ANSI_COLORS[c].rgb[2] - B);
        float d = sqrt( (double)((n1*n1)+(n2*n2)+(n3*n3)));
        if(d < smallest) {
            smallest = d;
            smallest_idx = c;
        }
    }
    // printf("%d idx: %d (%d, %d, %d) \n", smallest, smallest_idx,
    //     ANSI_COLORS[smallest_idx].rgb[0],
    //     ANSI_COLORS[smallest_idx].rgb[1],
    //     ANSI_COLORS[smallest_idx].rgb[2]);
    return ANSI_COLORS[smallest_idx].code;
}

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
    int termcode = color_serach((int)R, (int)G, (int)B);
    // setting RGB only works on limited terminals
    // printf(ESC_SET_ATTRIBUTE_MODE_BACKGROUND_RGB, R, G, B);
    printf(ESC_SET_ATTRIBUTE_MODE_1, termcode);
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
                if(use_color)
                    printf(ESC_SET_ATTRIBUTE_MODE_1, 0);
            }
            printf("\n");
        }
        free(output_pixels);
    }

    free_S(ASCIICHARS);
    stbi_image_free(image_data);
    return 0;
}
