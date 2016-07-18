#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <zlib.h>
#include <png.h>

#if __FreeBSD__
#include <sys/wait.h>
#endif

#if _WIN32
#define _bswap32(v) _byteswap_ulong(v)
#define _bswap16(v) _byteswap_ushort(v)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#define _bswap32(v) __builtin_bswap32(v)
#define _bswap16(v) __builtin_bswap16(v)
#endif

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: packbadge <in.png> <out.bin>\n");
        return 1;
    }

    /* Validate input */
    FILE* fp = fopen(argv[1], "rb");
    if (!fp)
    {
        fprintf(stderr, "'%s' is not able to be opened for reading as a regular file\n", argv[1]);
        return 1;
    }

    FILE* ofp = fopen(argv[2], "wb");
    if (!ofp)
    {
        fprintf(stderr, "'%s' is not able to be opened for writing as a regular file\n", argv[2]);
        return 1;
    }

    size_t decompSz = 0;
    int numMips = 1;

    z_stream z = {0};
    size_t rowSz = 0;
    uLong rowSzC = 0;
    png_bytep row;
    png_bytep rowC;

    /* Get PNG data */
    char header[8];
    fread(header, 1, 8, fp);
    if (png_sig_cmp((png_const_bytep)header, 0, 8))
    {
        fprintf(stderr, "invalid PNG signature in '%s'\n", argv[1]);
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
        fprintf(stderr, "unable to initialize libpng I/O for '%s'\n", argv[1]);
        fclose(fp);
        fclose(ofp);
        return 1;
    }

    png_init_io(pngRead, fp);
    png_set_sig_bytes(pngRead, 8);

    png_read_info(pngRead, info);

    png_uint_32 width = png_get_image_width(pngRead, info);
    png_uint_32 height = png_get_image_height(pngRead, info);
    decompSz = width * height * 4;
    png_byte colorType = png_get_color_type(pngRead, info);
    png_byte bitDepth = png_get_bit_depth(pngRead, info);

    if (colorType != PNG_COLOR_TYPE_RGB_ALPHA)
    {
        fprintf(stderr, "'%s' is not in RGBA color mode\n", argv[1]);
        fclose(fp);
        fclose(ofp);
        return 1;
    }

    if (bitDepth != 8)
    {
        fprintf(stderr, "'%s' is not 8 bits-per-channel\n", argv[1]);
        fclose(fp);
        fclose(ofp);
        return 1;
    }

    if (setjmp(png_jmpbuf(pngRead)))
    {
        fprintf(stderr, "unable to read image in '%s'\n", argv[1]);
        fclose(fp);
        fclose(ofp);
        return 1;
    }

    uint32_t fmt = _bswap32(16);
    uint16_t w = _bswap16(width);
    uint16_t h = _bswap16(height);
    uint32_t mips = _bswap32(numMips);
    uint32_t dsz = _bswap32(decompSz);
    fwrite(&fmt, 1, 4, ofp);
    fwrite(&w, 1, 2, ofp);
    fwrite(&h, 1, 2, ofp);
    fwrite(&mips, 1, 4, ofp);
    fwrite(&dsz, 1, 4, ofp);

    rowSz = width*4;
    rowSzC = compressBound(rowSz);
    deflateInit(&z, Z_DEFAULT_COMPRESSION);

    row = malloc(rowSz);
    rowC = malloc(rowSzC);

    for (png_uint_32 r=0 ; r<height ; ++r)
    {
        png_read_row(pngRead, row, NULL);
        z.next_in = row;
        z.avail_in = rowSz;
        while (z.avail_in)
        {
            z.next_out = rowC;
            z.avail_out = rowSzC;
            z.total_out = 0;
            deflate(&z, Z_NO_FLUSH);
            fwrite(rowC, 1, z.total_out, ofp);
        }
    }
    png_destroy_read_struct(&pngRead, &info, NULL);

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

    fclose(fp);
    return 0;
}
