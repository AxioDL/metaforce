#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>
#include <png.h>

static int CountBits(uint32_t n)
{
    int ret = 0;
    for (int i=0 ; i<32 ; ++i)
        if (((n >> i) & 1) != 0)
            ++ret;
    return ret;
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Usage: packicons <inkscape-bin> <in.svg> <out.bin>\n");
        return 1;
    }

    /* Validate inkscape */
    char command[2048];
    snprintf(command, 2048, "%s --version", argv[1]);
    FILE* fp = popen(command, "r");
    if (!fp)
    {
        fprintf(stderr, "'%s' is not executable on this system\n", command);
        return 1;
    }

    char readback[8];
    if (fread(readback, 1, 8, fp) != 8 || strncmp(readback, "Inkscape", 8))
    {
        fprintf(stderr, "'%s' did not return expected \"Inkscape\"\n", command);
        pclose(fp);
        return 1;
    }
    pclose(fp);

    /* Validate input */
    fp = fopen(argv[2], "rb");
    if (!fp)
    {
        fprintf(stderr, "'%s' is not able to be opened for reading as a regular file\n", argv[2]);
        return 1;
    }
    fclose(fp);

#ifdef _WIN32
    char* TMPDIR = getenv(L"TEMP");
    if (!TMPDIR)
        TMPDIR = (char*)"\\Temp";
#else
    char* TMPDIR = getenv("TMPDIR");
    if (!TMPDIR)
        TMPDIR = (char*)"/tmp";
#endif

    FILE* ofp = fopen(argv[3], "wb");
    if (!ofp)
    {
        fprintf(stderr, "'%s' is not able to be opened for writing as a regular file\n", argv[3]);
        return 1;
    }

    int numMips = 0;
    for (int i=512 ; i>=1 ; i/=2)
        ++numMips;

    z_stream z = {};
    size_t rowSz = 0;
    uLong rowSzC = 0;
    png_bytep row;
    png_bytep rowC;

    for (int i=512 ; i>=1 ; i/=2)
    {
        printf("Rendering icons @%dx%d\n", i, i);
        fflush(stdout);

        snprintf(command, 2048, "%s --export-png=\"%s/icon_pack.png\" --export-width=%d --export-height=%d \"%s\"",
                 argv[1], TMPDIR, i, i, argv[2]);
        fp = popen(command, "r");
        if (!fp)
        {
            fprintf(stderr, "'%s' is not executable on this system\n", command);
            fclose(ofp);
            return 1;
        }
        int status = pclose(fp);
        if (WEXITSTATUS(status))
        {
            fprintf(stderr, "'%s' failed\n", command);
            fclose(ofp);
            return 1;
        }

        /* Get PNG data */
        snprintf(command, 2048, "%s/icon_pack.png", TMPDIR);
        fp = fopen(command, "rb");
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

        if (CountBits(width) != 1 || CountBits(height) != 1)
        {
            fprintf(stderr, "'%s' is not power-of-2 in one or both dimensions\n", command);
            fclose(fp);
            fclose(ofp);
            return 1;
        }

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

        if (i == 512)
        {
            uint32_t fmt = htonl(16);
            uint16_t w = htons(width);
            uint16_t h = htons(height);
            uint32_t mips = htonl(numMips);
            fwrite(&fmt, 1, 4, ofp);
            fwrite(&w, 1, 2, ofp);
            fwrite(&h, 1, 2, ofp);
            fwrite(&mips, 1, 4, ofp);

            rowSz = width*4;
            rowSzC = compressBound(rowSz);
            deflateInit(&z, Z_DEFAULT_COMPRESSION);
            row = malloc(rowSz);
            rowC = malloc(rowSzC);
        }

        for (int r=0 ; r<height ; ++r)
        {
            png_read_row(pngRead, row, NULL);
            z.next_in = row;
            z.avail_in = rowSz;
            z.next_out = rowC;
            z.avail_out = rowSzC;
            z.total_out = 0;
            deflate(&z, Z_NO_FLUSH);
            fwrite(rowC, 1, z.total_out, ofp);
        }
        rowSz /= 2;

        png_destroy_read_struct(&pngRead, &info, NULL);
        fclose(fp);
    }

    if (rowSzC)
    {
        int finishCycle = Z_OK;
        while (finishCycle != Z_STREAM_END)
        {
            z.next_out = rowC;
            z.avail_out = rowSzC;
            z.total_out = 0;
            finishCycle = deflate(&z, Z_FINISH);
            fwrite(rowC, 1, z.total_out, ofp);
        }
        deflateEnd(&z);
        free(row);
        free(rowC);
    }

    fclose(ofp);
    return 0;
}
