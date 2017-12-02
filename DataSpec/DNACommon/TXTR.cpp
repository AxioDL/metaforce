#include <png.h>
#include <squish.h>
#include "TXTR.hpp"
#include "PAK.hpp"
#include "athena/FileWriter.hpp"

namespace DataSpec
{

static logvisor::Module Log("libpng");

static int CountBits(uint32_t n)
{
    int ret = 0;
    for (int i=0 ; i<32 ; ++i)
        if (((n >> i) & 1) != 0)
            ++ret;
    return ret;
}

/* Box filter algorithm (for mipmapping) */
static void BoxFilter(const uint8_t* input, unsigned chanCount,
                      unsigned inWidth, unsigned inHeight, uint8_t* output)
{
    unsigned mipWidth = 1;
    unsigned mipHeight = 1;
    if (inWidth > 1)
        mipWidth = inWidth / 2;
    if (inHeight > 1)
        mipHeight = inHeight / 2;

    unsigned y,x,c;
    for (y=0 ; y<mipHeight ; ++y)
    {
        unsigned miplineBase = mipWidth * y;
        unsigned in1LineBase = inWidth * (y*2);
        unsigned in2LineBase = inWidth * (y*2+1);
        for (x=0 ; x<mipWidth ; ++x)
        {
            uint8_t* out = &output[(miplineBase+x)*chanCount];
            for (c=0 ; c<chanCount ; ++c)
            {
                uint32_t tmp = 0;
                tmp += input[(in1LineBase+(x*2))*chanCount+c];
                tmp += input[(in1LineBase+(x*2+1))*chanCount+c];
                tmp += input[(in2LineBase+(x*2))*chanCount+c];
                tmp += input[(in2LineBase+(x*2+1))*chanCount+c];
                out[c] = tmp / 4;
            }
        }
    }
}

static size_t ComputeMippedTexelCount(unsigned inWidth, unsigned inHeight)
{
    size_t ret = 0;
    while (inWidth > 0 && inHeight > 0)
    {
        ret += inWidth * inHeight;
        inWidth /= 2;
        inHeight /= 2;
    }
    return ret;
}

/* GX uses this upsampling technique to extract full 8-bit range */
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
    return btexels[ry*4+rx/2] >> ((rx&1)?0:4) & 0xf;
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
    uint16_t ar = hecl::SBig(artexels[ry*4+rx]);
    *a = ar >> 8 & 0xff;
    *r = ar & 0xff;
    uint16_t gb = hecl::SBig(gbtexels[ry*4+rx]);
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
    uint32_t format = hecl::SBig(*(uint32_t*)data);
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
            uint16_t texel = hecl::SBig(data16[e]);
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
            uint16_t texel = hecl::SBig(data16[e]);
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
    uint32_t format = hecl::SBig(*(uint32_t*)data);
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
            uint16_t texel = hecl::SBig(data16[e]);
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
            uint16_t texel = hecl::SBig(data16[e]);
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

static void C4Palette(png_structrp png, png_infop info)
{
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
    png_set_IHDR(png, info, width, height, 8,
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
            uint16_t texel = hecl::SBig(Lookup16BPP(texels, width, x, y));
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
            uint16_t texel = hecl::SBig(Lookup16BPP(texels, width, x, y));
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
    Log.report(logvisor::Error, msg);
}

static void PNGWarn(png_structp png, png_const_charp msg)
{
    Log.report(logvisor::Warning, msg);
}

bool TXTR::Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
{
    uint32_t format = rs.readUint32Big();
    uint16_t width = rs.readUint16Big();
    uint16_t height = rs.readUint16Big();
    uint32_t numMips = rs.readUint32Big();

    FILE* fp = hecl::Fopen(outPath.getAbsolutePath().data(), _S("wb"));
    if (!fp)
    {
        Log.report(logvisor::Error,
                   _S("Unable to open '%s' for writing"),
                   outPath.getAbsolutePath().data());
        return false;
    }
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, PNGErr, PNGWarn);
    png_init_io(png, fp);
    png_infop info = png_create_info_struct(png);

    png_text textStruct = {};
    textStruct.key = png_charp("urde_nomip");
    if (numMips == 1)
        png_set_text(png, info, &textStruct, 1);

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

static std::unique_ptr<uint8_t[]> ReadPalette(png_structp png, png_infop info, size_t& szOut)
{
    std::unique_ptr<uint8_t[]> ret;
    png_sPLT_tp palettes;
    int paletteCount = png_get_sPLT(png, info, &palettes);
    if (paletteCount)
    {
        for (int i=0 ; i<paletteCount ; ++i)
        {
            png_sPLT_tp palette = &palettes[i];
            if (!strncmp(palette->name, "GX_", 3))
            {
                if (palette->nentries > 16)
                {
                    /* This is a C8 palette */
                    ret.reset(new uint8_t[4 * 257]);
                    szOut = 4 * 257;
                    *reinterpret_cast<uint32_t*>(ret.get()) = hecl::SBig(256);
                    uint8_t* cur = ret.get() + 4;
                    for (int j=0 ; j<256 ; ++j)
                    {
                        if (j < palette->nentries)
                        {
                            png_sPLT_entryp entry = &palette->entries[j];
                            if (palette->depth == 16)
                            {
                                *cur++ = entry->red >> 8;
                                *cur++ = entry->green >> 8;
                                *cur++ = entry->blue >> 8;
                                *cur++ = entry->alpha >> 8;
                            }
                            else
                            {
                                *cur++ = entry->red;
                                *cur++ = entry->green;
                                *cur++ = entry->blue;
                                *cur++ = entry->alpha;
                            }
                        }
                        else
                        {
                            *cur++ = 0;
                            *cur++ = 0;
                            *cur++ = 0;
                            *cur++ = 0;
                        }
                    }
                }
                else
                {
                    /* This is a C4 palette */
                    ret.reset(new uint8_t[4 * 17]);
                    szOut = 4 * 17;
                    *reinterpret_cast<uint32_t*>(ret.get()) = hecl::SBig(16);
                    uint8_t* cur = ret.get() + 4;
                    for (int j=0 ; j<16 ; ++j)
                    {
                        if (j < palette->nentries)
                        {
                            png_sPLT_entryp entry = &palette->entries[j];
                            if (palette->depth == 16)
                            {
                                *cur++ = entry->red >> 8;
                                *cur++ = entry->green >> 8;
                                *cur++ = entry->blue >> 8;
                                *cur++ = entry->alpha >> 8;
                            }
                            else
                            {
                                *cur++ = entry->red;
                                *cur++ = entry->green;
                                *cur++ = entry->blue;
                                *cur++ = entry->alpha;
                            }
                        }
                        else
                        {
                            *cur++ = 0;
                            *cur++ = 0;
                            *cur++ = 0;
                            *cur++ = 0;
                        }
                    }
                }
                break;
            }
        }
    }
    else
    {
        png_colorp palettes;
        int colorCount;
        if (png_get_PLTE(png, info, &palettes, &colorCount) == PNG_INFO_PLTE)
        {
            if (colorCount > 16)
            {
                /* This is a C8 palette */
                ret.reset(new uint8_t[4 * 257]);
                szOut = 4 * 257;
                *reinterpret_cast<uint32_t*>(ret.get()) = hecl::SBig(256);
                uint8_t* cur = ret.get() + 4;
                for (int j=0 ; j<256 ; ++j)
                {
                    if (j < colorCount)
                    {
                        png_colorp entry = &palettes[j];
                        *cur++ = entry->red;
                        *cur++ = entry->green;
                        *cur++ = entry->blue;
                        *cur++ = 0xff;
                    }
                    else
                    {
                        *cur++ = 0;
                        *cur++ = 0;
                        *cur++ = 0;
                        *cur++ = 0;
                    }
                }
            }
            else
            {
                /* This is a C4 palette */
                ret.reset(new uint8_t[4 * 17]);
                szOut = 4 * 17;
                *reinterpret_cast<uint32_t*>(ret.get()) = hecl::SBig(16);
                uint8_t* cur = ret.get() + 4;
                for (int j=0 ; j<16 ; ++j)
                {
                    if (j < colorCount)
                    {
                        png_colorp entry = &palettes[j];
                        *cur++ = entry->red;
                        *cur++ = entry->green;
                        *cur++ = entry->blue;
                        *cur++ = 0xff;
                    }
                    else
                    {
                        *cur++ = 0;
                        *cur++ = 0;
                        *cur++ = 0;
                        *cur++ = 0;
                    }
                }
            }
        }
    }
    return ret;
}

bool TXTR::Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath)
{
    return false;
}

bool TXTR::CookPC(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath)
{
    FILE* inf = hecl::Fopen(inPath.getAbsolutePath().data(), _S("rb"));
    if (!inf)
    {
        Log.report(logvisor::Error,
                   _S("Unable to open '%s' for reading"),
                   inPath.getAbsolutePath().data());
        return false;
    }

    /* Validate PNG */
    char header[8];
    fread(header, 1, 8, inf);
    if (png_sig_cmp((png_const_bytep)header, 0, 8))
    {
        Log.report(logvisor::Error, _S("invalid PNG signature in '%s'"),
                   inPath.getAbsolutePath().data());
        fclose(inf);
        return false;
    }

    /* Setup PNG reader */
    png_structp pngRead = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!pngRead)
    {
        Log.report(logvisor::Error, "unable to initialize libpng");
        fclose(inf);
        return false;
    }
    png_infop info = png_create_info_struct(pngRead);
    if (!info)
    {
        Log.report(logvisor::Error, "unable to initialize libpng info");
        fclose(inf);
        png_destroy_read_struct(&pngRead, nullptr, nullptr);
        return false;
    }

    if (setjmp(png_jmpbuf(pngRead)))
    {
        Log.report(logvisor::Error, _S("unable to initialize libpng I/O for '%s'"),
                   inPath.getAbsolutePath().data());
        fclose(inf);
        png_destroy_read_struct(&pngRead, &info, nullptr);
        return false;
    }

    png_init_io(pngRead, inf);
    png_set_sig_bytes(pngRead, 8);

    png_read_info(pngRead, info);

    png_uint_32 width = png_get_image_width(pngRead, info);
    png_uint_32 height = png_get_image_height(pngRead, info);
    png_byte colorType = png_get_color_type(pngRead, info);
    png_byte bitDepth = png_get_bit_depth(pngRead, info);

    /* Disable mipmapping if urde_nomip embedded */
    bool mipmap = true;
    png_text* textStruct;
    int numText;
    png_get_text(pngRead, info, &textStruct, &numText);
    for (int i=0 ; i<numText ; ++i)
        if (!strcmp(textStruct[i].key, "urde_nomip"))
            mipmap = false;

    /* Compute mipmap levels */
    size_t numMips = 1;
    if (mipmap && CountBits(width) == 1 && CountBits(height) == 1)
    {
        size_t index = std::min(width, height);
        while (index >>= 1) ++numMips;
    }

    if (bitDepth != 8)
    {
        Log.report(logvisor::Error, _S("'%s' is not 8 bits-per-channel"),
                   inPath.getAbsolutePath().data());
        fclose(inf);
        png_destroy_read_struct(&pngRead, &info, nullptr);
        return false;
    }

    std::unique_ptr<uint8_t[]> paletteBuf;
    size_t paletteSize = 0;

    size_t rowSize = 0;
    size_t nComps = 4;
    switch (colorType)
    {
    case PNG_COLOR_TYPE_GRAY:
        rowSize = width;
        break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
        rowSize = width * 2;
        break;
    case PNG_COLOR_TYPE_RGB:
        rowSize = width * 3;
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        rowSize = width * 4;
        break;
    case PNG_COLOR_TYPE_PALETTE:
        rowSize = width;
        nComps = 1;
        paletteBuf = ReadPalette(pngRead, info, paletteSize);
        break;
    default:
        Log.report(logvisor::Error, _S("unsupported color type in '%s'"),
                   inPath.getAbsolutePath().data());
        fclose(inf);
        png_destroy_read_struct(&pngRead, &info, nullptr);
        return false;
    }

    /* Intermediate row-read buf (file components) */
    std::unique_ptr<uint8_t[]> rowBuf(new uint8_t[rowSize]);

    /* Final mipmapped buf (RGBA components) */
    std::unique_ptr<uint8_t[]> bufOut;
    size_t bufLen = 0;
    if (numMips > 1)
        bufLen = ComputeMippedTexelCount(width, height) * nComps;
    else
        bufLen = width * height * nComps;
    bufOut.reset(new uint8_t[bufLen]);

    if (setjmp(png_jmpbuf(pngRead)))
    {
        Log.report(logvisor::Error, _S("unable to read image in '%s'"),
                   inPath.getAbsolutePath().data());
        fclose(inf);
        png_destroy_read_struct(&pngRead, &info, nullptr);
        return false;
    }

    /* Track alpha values for DXT1 eligibility */
    bool doDXT1 = (colorType == PNG_COLOR_TYPE_RGB || colorType == PNG_COLOR_TYPE_RGB_ALPHA) &&
                  width >= 4 && height >= 4;

    /* Read and make RGBA */
    for (int r=height-1 ; r>=0 ; --r)
    {
        png_read_row(pngRead, rowBuf.get(), nullptr);
        switch (colorType)
        {
        case PNG_COLOR_TYPE_GRAY:
            for (unsigned i=0 ; i<width ; ++i)
            {
                size_t outbase = (r*width+i)*4;
                bufOut[outbase] = rowBuf[i];
                bufOut[outbase+1] = rowBuf[i];
                bufOut[outbase+2] = rowBuf[i];
                bufOut[outbase+3] = rowBuf[i];
            }
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            for (unsigned i=0 ; i<width ; ++i)
            {
                size_t inbase = i*2;
                size_t outbase = (r*width+i)*4;
                bufOut[outbase] = rowBuf[inbase];
                bufOut[outbase+1] = rowBuf[inbase];
                bufOut[outbase+2] = rowBuf[inbase];
                bufOut[outbase+3] = rowBuf[inbase+1];
            }
            break;
        case PNG_COLOR_TYPE_RGB:
            for (unsigned i=0 ; i<width ; ++i)
            {
                size_t inbase = i*3;
                size_t outbase = (r*width+i)*4;
                bufOut[outbase] = rowBuf[inbase];
                bufOut[outbase+1] = rowBuf[inbase+1];
                bufOut[outbase+2] = rowBuf[inbase+2];
                bufOut[outbase+3] = 0xff;
            }
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            for (unsigned i=0 ; i<width ; ++i)
            {
                size_t inbase = i*4;
                size_t outbase = (r*width+i)*4;
                bufOut[outbase] = rowBuf[inbase];
                bufOut[outbase+1] = rowBuf[inbase+1];
                bufOut[outbase+2] = rowBuf[inbase+2];
                bufOut[outbase+3] = rowBuf[inbase+3];
                if (rowBuf[inbase+3] != 0 && rowBuf[inbase+3] != 255)
                    doDXT1 = false;
            }
            break;
        case PNG_COLOR_TYPE_PALETTE:
            for (unsigned i=0 ; i<width ; ++i)
                bufOut[r*width+i] = rowBuf[i];
            break;
        default: break;
        }
    }

    png_destroy_read_struct(&pngRead, &info, nullptr);
    fclose(inf);

    /* Perform box-filter mipmap */
    if (numMips > 1)
    {
        const uint8_t* filterIn = bufOut.get();
        uint8_t* filterOut = bufOut.get() + width * height * nComps;
        unsigned filterWidth = width;
        unsigned filterHeight = height;
        for (size_t i=1 ; i<numMips ; ++i)
        {
            BoxFilter(filterIn, nComps, filterWidth, filterHeight, filterOut);
            filterIn += filterWidth * filterHeight * nComps;
            filterWidth /= 2;
            filterHeight /= 2;
            filterOut += filterWidth * filterHeight * nComps;
        }
    }

    /* Do DXT1 compression */
    std::unique_ptr<uint8_t[]> compOut;
    size_t compLen = 0;
    if (doDXT1)
    {
        int filterWidth = width;
        int filterHeight = height;
        size_t i;
        for (i=0 ; i<numMips ; ++i)
        {
            compLen += squish::GetStorageRequirements(filterWidth, filterHeight, squish::kDxt1);
            if (filterWidth == 4 || filterHeight == 4)
            {
                ++i;
                break;
            }
            filterWidth /= 2;
            filterHeight /= 2;
        }
        numMips = i;

        compOut.reset(new uint8_t[compLen]);

        filterWidth = width;
        filterHeight = height;
        const uint8_t* rgbaIn = bufOut.get();
        uint8_t* blocksOut = compOut.get();
        for (i=0 ; i<numMips ; ++i)
        {
            int thisLen = squish::GetStorageRequirements(filterWidth, filterHeight, squish::kDxt1);
            squish::CompressImage(rgbaIn, filterWidth, filterHeight, blocksOut, squish::kDxt1);
            rgbaIn += filterWidth * filterHeight * nComps;
            blocksOut += thisLen;
            filterWidth /= 2;
            filterHeight /= 2;
        }
    }

    /* Do write out */
    athena::io::FileWriter outf(outPath.getAbsolutePath(), true, false);
    if (outf.hasError())
    {
        Log.report(logvisor::Error,
                   _S("Unable to open '%s' for writing"),
                   outPath.getAbsolutePath().data());
        return false;
    }

    int format;
    if (paletteBuf && paletteSize)
        format = 17;
    else if (compOut)
        format = 18;
    else
        format = 16;
    outf.writeInt32Big(format);
    outf.writeInt16Big(width);
    outf.writeInt16Big(height);
    outf.writeInt32Big(numMips);
    if (paletteBuf && paletteSize)
        outf.writeUBytes(paletteBuf.get(), paletteSize);
    if (compOut)
        outf.writeUBytes(compOut.get(), compLen);
    else
        outf.writeUBytes(bufOut.get(), bufLen);

    return true;
}

}
