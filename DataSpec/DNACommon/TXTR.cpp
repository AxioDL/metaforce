#include <png.h>
#include <squish.h>
#include "TXTR.hpp"
#include "PAK.hpp"

namespace Retro
{

static LogVisor::LogModule Log("libpng");

/* GX uses this upsampling technique to prevent banding on downsampled texture formats */
static inline uint8_t Convert3To8(uint8_t v)
{
    /* Swizzle bits: 00000123 -> 12312312 */
    return (v << 5) | (v << 2) | (v >> 1);
}

static inline uint8_t Convert4To8(uint8_t v)
{
    /* Swizzle bits: 00001234 -> 12341234 */
    return (v << 4) | v;
}

static inline uint8_t Convert5To8(uint8_t v)
{
    /* Swizzle bits: 00012345 -> 12345123 */
    return (v << 3) | (v >> 2);
}

static inline uint8_t Convert6To8(uint8_t v)
{
    /* Swizzle bits: 00123456 -> 12345612 */
    return (v << 2) | (v >> 4);
}

static inline uint8_t Lookup4BPP(const uint8_t* texels, int width, int x, int y)
{
    int bwidth = (width + 7) / 8;
    int bx = x / 8;
    int by = y / 8;
    int rx = x % 8;
    int ry = y % 8;
    int bidx = by * bwidth + bx;
    const uint8_t* btexels = &texels[32*bidx];
    return btexels[ry*4+rx/2] >> ((rx%2)?0:4) & 0xf;
}

static inline uint8_t Lookup8BPP(const uint8_t* texels, int width, int x, int y)
{
    int bwidth = (width + 7) / 8;
    int bx = x / 8;
    int by = y / 4;
    int rx = x % 8;
    int ry = y % 4;
    int bidx = by * bwidth + bx;
    const uint8_t* btexels = &texels[32*bidx];
    return btexels[ry*8+rx];
}

static inline uint16_t Lookup16BPP(const uint8_t* texels, int width, int x, int y)
{
    int bwidth = (width + 3) / 4;
    int bx = x / 4;
    int by = y / 4;
    int rx = x % 4;
    int ry = y % 4;
    int bidx = by * bwidth + bx;
    const uint16_t* btexels = (uint16_t*)&texels[32*bidx];
    return btexels[ry*4+rx];
}

static inline void LookupRGBA8(const uint8_t* texels, int width, int x, int y,
                               uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a)
{
    int bwidth = (width + 3) / 4;
    int bx = x / 4;
    int by = y / 4;
    int rx = x % 4;
    int ry = y % 4;
    int bidx = (by * bwidth + bx) * 2;
    const uint16_t* artexels = (uint16_t*)&texels[32*bidx];
    const uint16_t* gbtexels = (uint16_t*)&texels[32*(bidx+1)];
    uint16_t ar = HECL::SBig(artexels[ry*4+rx]);
    *a = ar >> 8 & 0xff;
    *r = ar & 0xff;
    uint16_t gb = HECL::SBig(gbtexels[ry*4+rx]);
    *g = gb >> 8 & 0xff;
    *b = gb & 0xff;
}

static void DecodeI4(png_structrp png, png_infop info,
                     const uint8_t* texels, int width, int height)
{
    png_set_IHDR(png, info, width, height, 8,
                 PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    std::unique_ptr<uint8_t[]> buf(new uint8_t[width]);
    //memset(buf.get(), 0, width);
    for (int y=height-1 ; y>=0 ; --y)
    {
        for (int x=0 ; x<width ; ++x)
            buf[x] = Convert4To8(Lookup4BPP(texels, width, x, y));
        png_write_row(png, buf.get());
    }
}

static void DecodeI8(png_structrp png, png_infop info,
                     const uint8_t* texels, int width, int height)
{
    png_set_IHDR(png, info, width, height, 8,
                 PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    std::unique_ptr<uint8_t[]> buf(new uint8_t[width]);
    for (int y=height-1 ; y>=0 ; --y)
    {
        for (int x=0 ; x<width ; ++x)
            buf[x] = Lookup8BPP(texels, width, x, y);
        png_write_row(png, buf.get());
    }
}

static void DecodeIA4(png_structrp png, png_infop info,
                      const uint8_t* texels, int width, int height)
{
    png_set_IHDR(png, info, width, height, 8,
                 PNG_COLOR_TYPE_GRAY_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    std::unique_ptr<uint8_t[]> buf(new uint8_t[width*2]);
    for (int y=height-1 ; y>=0 ; --y)
    {
        for (int x=0 ; x<width ; ++x)
        {
            uint8_t texel = Lookup8BPP(texels, width, x, y);
            buf[x*2] = texel >> 4 & 0xf;
            buf[x*2+1] = texel & 0xf;
        }
        png_write_row(png, buf.get());
    }
}

static void DecodeIA8(png_structrp png, png_infop info,
                      const uint8_t* texels, int width, int height)
{
    png_set_IHDR(png, info, width, height, 8,
                 PNG_COLOR_TYPE_GRAY_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    std::unique_ptr<uint16_t[]> buf(new uint16_t[width]);
    for (int y=height-1 ; y>=0 ; --y)
    {
        for (int x=0 ; x<width ; ++x)
            buf[x] = Lookup16BPP(texels, width, x, y);
        png_write_row(png, (png_bytep)buf.get());
    }
}

static const uint8_t* DecodePalette(png_structrp png, png_infop info,
                                    int numEntries, const uint8_t* data)
{
    uint32_t format = HECL::SBig(*(uint32_t*)data);
    data += 8;
    png_color cEntries[256];
    png_byte aEntries[256];
    switch (format)
    {
    case 0:
    {
        /* IA8 */
        for (int e=0 ; e<numEntries ; ++e)
        {
            cEntries[e].red = data[e*2];
            cEntries[e].green = data[e*2];
            cEntries[e].blue = data[e*2];
            aEntries[e] = data[e*2+1];
        }
        break;
    }
    case 1:
    {
        /* RGB565 */
        const uint16_t* data16 = (uint16_t*)data;
        for (int e=0 ; e<numEntries ; ++e)
        {
            uint16_t texel = HECL::SBig(data16[e]);
            cEntries[e].red = Convert5To8(texel >> 11 & 0x1f);
            cEntries[e].green = Convert6To8(texel >> 5 & 0x3f);
            cEntries[e].blue = Convert5To8(texel & 0x1f);
        }
        break;
    }
    case 2:
    {
        /* RGB5A3 */
        const uint16_t* data16 = (uint16_t*)data;
        for (int e=0 ; e<numEntries ; ++e)
        {
            uint16_t texel = HECL::SBig(data16[e]);
            if (texel & 0x8000)
            {
                cEntries[e].red = Convert5To8(texel >> 10 & 0x1f);
                cEntries[e].green = Convert5To8(texel >> 5 & 0x1f);
                cEntries[e].blue = Convert5To8(texel & 0x1f);
                aEntries[e] = 0xff;
            }
            else
            {
                cEntries[e].red = Convert4To8(texel >> 8 & 0xf);
                cEntries[e].green = Convert4To8(texel >> 4 & 0xf);
                cEntries[e].blue = Convert4To8(texel & 0xf);
                aEntries[e] = Convert3To8(texel >> 12 & 0x7);
            }
        }
        break;
    }
    }
    png_set_PLTE(png, info, cEntries, numEntries);
    if (format == 0 || format == 2)
        png_set_tRNS(png, info, aEntries, numEntries, nullptr);
    data += numEntries * 2;
    return data;
}

static const uint8_t* DecodePaletteSPLT(png_structrp png, png_infop info,
                                        int numEntries, const uint8_t* data)
{
    uint32_t format = HECL::SBig(*(uint32_t*)data);
    data += 8;
    png_sPLT_entry entries[256] = {};
    png_sPLT_t GXEntry =
    {
        (char*)"GXPalette",
        8,
        entries,
        numEntries
    };
    switch (format)
    {
    case 0:
    {
        /* IA8 */
        GXEntry.name = (char*)"GX_IA8";
        for (int e=0 ; e<numEntries ; ++e)
        {
            entries[e].red = data[e*2];
            entries[e].green = data[e*2];
            entries[e].blue = data[e*2];
            entries[e].alpha = data[e*2+1];
        }
        break;
    }
    case 1:
    {
        /* RGB565 */
        GXEntry.name = (char*)"GX_RGB565";
        const uint16_t* data16 = (uint16_t*)data;
        for (int e=0 ; e<numEntries ; ++e)
        {
            uint16_t texel = HECL::SBig(data16[e]);
            entries[e].red = Convert5To8(texel >> 11 & 0x1f);
            entries[e].green = Convert6To8(texel >> 5 & 0x3f);
            entries[e].blue = Convert5To8(texel & 0x1f);
            entries[e].alpha = 0xff;
        }
        break;
    }
    case 2:
    {
        /* RGB5A3 */
        GXEntry.name = (char*)"GX_RGB5A3";
        const uint16_t* data16 = (uint16_t*)data;
        for (int e=0 ; e<numEntries ; ++e)
        {
            uint16_t texel = HECL::SBig(data16[e]);
            if (texel & 0x8000)
            {
                entries[e].red = Convert5To8(texel >> 10 & 0x1f);
                entries[e].green = Convert5To8(texel >> 5 & 0x1f);
                entries[e].blue = Convert5To8(texel & 0x1f);
                entries[e].alpha = 0xff;
            }
            else
            {
                entries[e].red = Convert4To8(texel >> 8 & 0xf);
                entries[e].green = Convert4To8(texel >> 4 & 0xf);
                entries[e].blue = Convert4To8(texel & 0xf);
                entries[e].alpha = Convert3To8(texel >> 12 & 0x7);
            }
        }
        break;
    }
    }
    png_set_sPLT(png, info, &GXEntry, 1);
    data += numEntries * 2;
    return data;
}

static void C4Palette(png_structrp png, png_infop info)
{
    static const png_color C4Colors[] =
    {
        {0,0,0},
        {155,0,0},
        {0,155,0},
        {0,0,155},
        {155,155,0},
        {155,0,155},
        {0,155,155},
        {155,155,155},
        {55,55,55},
        {255,0,0},
        {0,255,0},
        {0,0,255},
        {255,255,0},
        {255,0,255},
        {0,255,255},
        {255,255,255}
    };
    png_set_PLTE(png, info, C4Colors, 16);
}

static void DecodeC4(png_structrp png, png_infop info,
                     const uint8_t* data, int width, int height)
{
    png_set_IHDR(png, info, width, height, 8,
                 PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    C4Palette(png, info);
    const uint8_t* texels = DecodePaletteSPLT(png, info, 16, data);
    png_write_info(png, info);
    std::unique_ptr<uint8_t[]> buf(new uint8_t[width]);
    for (int y=0 ; y<height ; ++y)
    {
        for (int x=0 ; x<width ; ++x)
            buf[x] = Lookup4BPP(texels, width, x, y);
        png_write_row(png, buf.get());
    }
}


static void DecodeC8(png_structrp png, png_infop info,
                     const uint8_t* data, int width, int height)
{
    png_set_IHDR(png, info, width, height, 4,
                 PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    const uint8_t* texels = DecodePalette(png, info, 256, data);
    png_write_info(png, info);
    std::unique_ptr<uint8_t[]> buf(new uint8_t[width]);
    for (int y=0 ; y<height ; ++y)
    {
        for (int x=0 ; x<width ; ++x)
            buf[x] = Lookup8BPP(texels, width, x, y);
        png_write_row(png, buf.get());
    }
}

static void DecodeRGB565(png_structrp png, png_infop info,
                         const uint8_t* texels, int width, int height)
{
    png_set_IHDR(png, info, width, height, 8,
                 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    std::unique_ptr<uint8_t[]> buf(new uint8_t[width*3]);
    for (int y=height-1 ; y>=0 ; --y)
    {
        for (int x=0 ; x<width ; ++x)
        {
            uint16_t texel = HECL::SBig(Lookup16BPP(texels, width, x, y));
            buf[x*3] = Convert5To8(texel >> 11 & 0x1f);
            buf[x*3+1] = Convert6To8(texel >> 5 & 0x3f);
            buf[x*3+2] = Convert5To8(texel & 0x1f);
        }
        png_write_row(png, buf.get());
    }
}

static void DecodeRGB5A3(png_structrp png, png_infop info,
                         const uint8_t* texels, int width, int height)
{
    png_set_IHDR(png, info, width, height, 8,
                 PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    std::unique_ptr<uint8_t[]> buf(new uint8_t[width*4]);
    for (int y=height-1 ; y>=0 ; --y)
    {
        for (int x=0 ; x<width ; ++x)
        {
            uint16_t texel = HECL::SBig(Lookup16BPP(texels, width, x, y));
            if (texel & 0x8000)
            {
                buf[x*4] = Convert5To8(texel >> 10 & 0x1f);
                buf[x*4+1] = Convert5To8(texel >> 5 & 0x1f);
                buf[x*4+2] = Convert5To8(texel & 0x1f);
                buf[x*4+3] = 0xff;
            }
            else
            {
                buf[x*4] = Convert4To8(texel >> 8 & 0xf);
                buf[x*4+1] = Convert4To8(texel >> 4 & 0xf);
                buf[x*4+2] = Convert4To8(texel & 0xf);
                buf[x*4+3] = Convert3To8(texel >> 12 & 0x7);
            }
        }
        png_write_row(png, buf.get());
    }
}

static void DecodeRGBA8(png_structrp png, png_infop info,
                        const uint8_t* texels, int width, int height)
{
    png_set_IHDR(png, info, width, height, 8,
                 PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    std::unique_ptr<uint8_t[]> buf(new uint8_t[width*4]);
    for (int y=height-1 ; y>=0 ; --y)
    {
        for (int x=0 ; x<width ; ++x)
            LookupRGBA8(texels, width, x, y, &buf[x*4], &buf[x*4+1], &buf[x*4+2], &buf[x*4+3]);
        png_write_row(png, buf.get());
    }
}

struct DXTBlock
{
    uint16_t color1;
    uint16_t color2;
    uint8_t lines[4];
};

static void DecodeCMPR(png_structrp png, png_infop info,
                       const uint8_t* texels, int width, int height)
{
    png_set_IHDR(png, info, width, height, 8,
                 PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    /* Decode 8 rows at a time */
    int bwidth = (width + 7) / 8;
    int bpwidth = bwidth * 8;
    std::unique_ptr<uint32_t[]> buf(new uint32_t[bpwidth*8]);
    uint32_t* bTargets[4] = {
        buf.get(),
        buf.get() + 4,
        buf.get() + 4 * width,
        buf.get() + 4 * width + 4
    };
    for (int y=height/8-1 ; y>=0 ; --y)
    {
        const DXTBlock* blks = (DXTBlock*)(texels + 32 * bwidth * y);
        for (int x=0 ; x<width ; x+=8)
        {
            uint32_t blkOut[4][4][4];
            squish::Decompress((uint8_t*)blkOut[0][0], blks++, squish::kDxt1GCN);
            squish::Decompress((uint8_t*)blkOut[1][0], blks++, squish::kDxt1GCN);
            squish::Decompress((uint8_t*)blkOut[2][0], blks++, squish::kDxt1GCN);
            squish::Decompress((uint8_t*)blkOut[3][0], blks++, squish::kDxt1GCN);

            for (int bt=0 ; bt<4 ; ++bt)
                for (int by=0 ; by<4 ; ++by)
                    memcpy(bTargets[bt] + x + width * by, blkOut[bt][by], 16);
        }
        for (int r=7 ; r>=0 ; --r)
            png_write_row(png, (png_bytep)(bTargets[0] + width * r));
    }
}

static void PNGErr(png_structp png, png_const_charp msg)
{
    Log.report(LogVisor::Error, msg);
}

static void PNGWarn(png_structp png, png_const_charp msg)
{
    Log.report(LogVisor::Warning, msg);
}

bool TXTR::Extract(PAKEntryReadStream& rs, const HECL::ProjectPath& outPath)
{
    uint32_t format = rs.readUint32Big();
    uint16_t width = rs.readUint16Big();
    uint16_t height = rs.readUint16Big();
    uint32_t numMips = rs.readUint32Big();

    FILE* fp = HECL::Fopen(outPath.getAbsolutePath().c_str(), _S("wb"));
    if (!fp)
    {
        Log.report(LogVisor::Error,
                   _S("Unable to open '%s' for writing"),
                   outPath.getAbsolutePath().c_str());
        return false;
    }
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, PNGErr, PNGWarn);
    png_init_io(png, fp);
    png_infop info = png_create_info_struct(png);

    switch (format)
    {
    case 0:
        DecodeI4(png, info, rs.data() + 12, width, height);
        break;
    case 1:
        DecodeI8(png, info, rs.data() + 12, width, height);
        break;
    case 2:
        DecodeIA4(png, info, rs.data() + 12, width, height);
        break;
    case 3:
        DecodeIA8(png, info, rs.data() + 12, width, height);
        break;
    case 4:
        DecodeC4(png, info, rs.data() + 12, width, height);
        break;
    case 5:
        DecodeC8(png, info, rs.data() + 12, width, height);
        break;
    case 7:
        DecodeRGB565(png, info, rs.data() + 12, width, height);
        break;
    case 8:
        DecodeRGB5A3(png, info, rs.data() + 12, width, height);
        break;
    case 9:
        DecodeRGBA8(png, info, rs.data() + 12, width, height);
        break;
    case 10:
        DecodeCMPR(png, info, rs.data() + 12, width, height);
        break;
    }

    png_write_end(png, info);
    png_write_flush(png);
    png_destroy_write_struct(&png, &info);
    fclose(fp);

    return true;
}

bool TXTR::Cook(const HECL::ProjectPath& inPath, const HECL::ProjectPath& outPath)
{
    return false;
}

}
