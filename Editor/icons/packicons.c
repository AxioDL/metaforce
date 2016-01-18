#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <zlib.h>
#include <png.h>

#if _WIN32
#define _bswap32(v) _byteswap_ulong(v)
#define _bswap16(v) _byteswap_ushort(v)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#define _bswap32(v) __builtin_bswap32(v)
#define _bswap16(v) __builtin_bswap16(v)
#endif

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
    FILE* fp;

#if _WIN32
    STARTUPINFOA sinfo = {sizeof(STARTUPINFOA)};

    HANDLE hChildStd_OUT_Rd = NULL;
    HANDLE hChildStd_OUT_Wr = NULL;

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT.
    if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0))
    {
        fprintf(stderr, "unable to CreatePipe\n");
        return 1;
    }

    // Ensure the read handle to the pipe for STDOUT is not inherited
    if (!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
    {
        fprintf(stderr, "unable to SetHandleInformation\n");
        return 1;
    }

    sinfo.hStdError = hChildStd_OUT_Wr;
    sinfo.hStdOutput = hChildStd_OUT_Wr;
    sinfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pinfo;
    if (!CreateProcessA(argv[1], " --version", NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &sinfo, &pinfo))
    {
        LPSTR messageBuffer = NULL;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
        fprintf(stderr, "unable to launch inkscape from %s: %s\n", argv[1], messageBuffer);
        return 1;
    }
    CloseHandle(hChildStd_OUT_Wr);
    CloseHandle(pinfo.hThread);

    char readback[8];
    DWORD bytesRead = 0;
    if (!ReadFile(hChildStd_OUT_Rd, readback, 8, &bytesRead, NULL) || bytesRead != 8 ||
        strncmp(readback, "Inkscape", 8))
    {
        fprintf(stderr, "'%s' did not return expected \"Inkscape\"\n", command);
        CloseHandle(hChildStd_OUT_Rd);
        CloseHandle(pinfo.hProcess);
        return 1;
    }
    CloseHandle(hChildStd_OUT_Rd);
    WaitForSingleObject(pinfo.hProcess, INFINITE);
    CloseHandle(pinfo.hProcess);

#else
    snprintf(command, 2048, "%s --version", argv[1]);
    fp = popen(command, "r");
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

#endif

    /* Validate input */
    fp = fopen(argv[2], "rb");
    if (!fp)
    {
        fprintf(stderr, "'%s' is not able to be opened for reading as a regular file\n", argv[2]);
        return 1;
    }
    fclose(fp);

#ifdef _WIN32
    char* TMPDIR = getenv("TEMP");
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

    z_stream z = {0};
    size_t rowSz = 0;
    uLong rowSzC = 0;
    png_bytep row;
    png_bytep rowC;

    for (int i=512 ; i>=1 ; i/=2)
    {
        printf("Rendering icons @%dx%d\n", i, i);
        fflush(stdout);

#if _WIN32
        snprintf(command, 2048, " --export-png=\"%s/icon_pack.png\" --export-width=%d --export-height=%d \"%s\"",
                 TMPDIR, i, i, argv[2]);

        STARTUPINFOA sinfo = {sizeof(STARTUPINFOA)};
        PROCESS_INFORMATION pinfo;
        if (!CreateProcessA(argv[1], command, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &sinfo, &pinfo))
        {
            LPSTR messageBuffer = NULL;
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
            fprintf(stderr, "unable to launch inkscape from %s: %s\n", argv[1], messageBuffer);
            return 1;
        }
        CloseHandle(pinfo.hThread);
        WaitForSingleObject(pinfo.hProcess, INFINITE);
        CloseHandle(pinfo.hProcess);

#else
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
#endif

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
            uint32_t fmt = _bswap32(16);
            uint16_t w = _bswap16(width);
            uint16_t h = _bswap16(height);
            uint32_t mips = _bswap32(numMips);
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

        for (png_uint_32 r=0 ; r<height ; ++r)
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
