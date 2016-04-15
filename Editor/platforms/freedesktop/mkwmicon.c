/* clang -o mkwmicon -lpng mkwmicon.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <png.h>

static int CountBits(uint32_t n)
{
    int ret = 0;
    for (int i=0 ; i<32 ; ++i)
        if (((n >> i) & 1) != 0)
            ++ret;
    return ret;
}

static const int DIMS[] =
{
    16,
    32,
    48,
    64,
    128,
    0
};

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: makewmicon <out.bin>\n");
        return 1;
    }

    FILE* ofp = fopen(argv[1], "wb");
    if (!ofp)
    {
        fprintf(stderr, "'%s' is not able to be opened for writing as a regular file\n", argv[1]);
        return 1;
    }

    png_bytep row = malloc(4 * 128);

    char command[2048];

    for (const int* d = DIMS ; *d != 0 ; ++d)
    {
        printf("Rendering main icon @%dx%d\n", *d, *d);
        fflush(stdout);

        snprintf(command, 2048, "%dx%d/apps/urde.png", *d, *d);
        FILE* fp = fopen(command, "rb");
        if (!fp)
        {
            fprintf(stderr, "unable to open '%s' for reading\n", command);
            fclose(ofp);
            return 1;
        }

        char header[8];
        fread(header, 1, 8, fp);
        if (png_sig_cmp((png_const_bytep)header, 0, 8))
        {
            fprintf(stderr, "invalid PNG signature in '%s'\n", command);
            fclose(fp);
            fclose(ofp);
            return 1;
        }

        png_structp pngRead = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!pngRead)
        {
            fprintf(stderr, "unable to initialize libpng\n");
            fclose(fp);
            fclose(ofp);
            return 1;
        }
        png_infop info = png_create_info_struct(pngRead);
        if (!info)
        {
            fprintf(stderr, "unable to initialize libpng info\n");
            fclose(fp);
            fclose(ofp);
            return 1;
        }

        if (setjmp(png_jmpbuf(pngRead)))
        {
            fprintf(stderr, "unable to initialize libpng I/O for '%s'\n", command);
            fclose(fp);
            fclose(ofp);
            return 1;
        }

        png_init_io(pngRead, fp);
        png_set_sig_bytes(pngRead, 8);

        png_read_info(pngRead, info);

        png_uint_32 width = png_get_image_width(pngRead, info);
        png_uint_32 height = png_get_image_height(pngRead, info);
        png_byte colorType = png_get_color_type(pngRead, info);
        png_byte bitDepth = png_get_bit_depth(pngRead, info);

        if (colorType != PNG_COLOR_TYPE_RGB_ALPHA)
        {
            fprintf(stderr, "'%s' is not in RGBA color mode\n", command);
            fclose(fp);
            fclose(ofp);
            return 1;
        }

        if (bitDepth != 8)
        {
            fprintf(stderr, "'%s' is not 8 bits-per-channel\n", command);
            fclose(fp);
            fclose(ofp);
            return 1;
        }

        if (setjmp(png_jmpbuf(pngRead)))
        {
            fprintf(stderr, "unable to read image in '%s'\n", command);
            fclose(fp);
            fclose(ofp);
            return 1;
        }

        unsigned long lWidth = width;
        unsigned long lHeight = height;
        fwrite(&lWidth, 1, sizeof(lWidth), ofp);
        fwrite(&lHeight, 1, sizeof(lHeight), ofp);
        for (png_uint_32 r=0 ; r<height ; ++r)
        {
            png_read_row(pngRead, row, NULL);
            for (int i=0 ; i<width ; ++i)
            {
                unsigned long px;
                px = row[i*4+2];
                px |= row[i*4+1] << 8;
                px |= row[i*4] << 16;
                px |= row[i*4+3] << 24;
                fwrite(&px, 1, sizeof(unsigned long), ofp);
            }
        }

        png_destroy_read_struct(&pngRead, &info, NULL);
        fclose(fp);
    }

    free(row);
    fclose(ofp);
    return 0;
}
