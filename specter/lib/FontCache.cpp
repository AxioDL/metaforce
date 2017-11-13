#ifndef NOMINMAX
#define NOMINMAX 1
#endif

#include "specter/FontCache.hpp"
#include "logvisor/logvisor.hpp"
#include <athena/MemoryReader.hpp>
#include <stdint.h>
#include <zlib.h>

#include FT_GZIP_H
#include FT_SYSTEM_H
#include FT_OUTLINE_H
#include <freetype/internal/internal.h>
#include <freetype/internal/ftstream.h>
#include <freetype/internal/tttypes.h>

extern "C" const uint8_t DROIDSANS_PERMISSIVE[];
extern "C" size_t DROIDSANS_PERMISSIVE_SZ;

extern "C" const uint8_t BMONOFONT[];
extern "C" size_t BMONOFONT_SZ;

extern "C" const uint8_t SPECTERCURVES[];
extern "C" size_t SPECTERCURVES_SZ;

extern "C" const FT_Driver_ClassRec tt_driver_class;

namespace specter
{
static logvisor::Module Log("specter::FontCache");

const FCharFilter AllCharFilter =
std::make_pair("all-glyphs", [](uint32_t)->bool
{return true;});

const FCharFilter LatinCharFilter =
std::make_pair("latin-glyphs", [](uint32_t c)->bool
{return c <= 0xff || ((c - 0x2200) <= (0x23FF - 0x2200));});

const FCharFilter LatinAndJapaneseCharFilter =
std::make_pair("latin-and-jp-glyphs", [](uint32_t c)->bool
{return LatinCharFilter.second(c) ||
((c - 0x2E00) <= (0x30FF - 0x2E00)) ||
((c - 0x4E00) <= (0x9FFF - 0x4E00)) ||
((c - 0xFF00) <= (0xFFEF - 0xFF00));});

FontTag::FontTag(std::string_view name, bool subpixel, float points, uint32_t dpi)
{
    XXH64_state_t st;
    XXH64_reset(&st, 0);
    XXH64_update(&st, name.data(), name.size());
    XXH64_update(&st, &subpixel, 1);
    XXH64_update(&st, &points, 4);
    XXH64_update(&st, &dpi, 4);
    m_hash = XXH64_digest(&st);
}

FreeTypeGZipMemFace::FreeTypeGZipMemFace(FT_Library lib, const uint8_t* data, size_t sz)
: m_lib(lib)
{
    m_comp.base = (unsigned char*)data;
    m_comp.size = sz;
    m_comp.memory = lib->memory;
}

void FreeTypeGZipMemFace::open()
{
    if (m_face)
        return;

    if (FT_Stream_OpenGzip(&m_decomp, &m_comp))
        Log.report(logvisor::Fatal, "unable to open FreeType gzip stream");

    FT_Open_Args args =
    {
        FT_OPEN_STREAM,
        nullptr,
        0,
        nullptr,
        &m_decomp
    };

    if (FT_Open_Face(m_lib, &args, 0, &m_face))
        Log.report(logvisor::Fatal, "unable to open FreeType gzip face");
}

void FreeTypeGZipMemFace::close()
{
    if (!m_face)
        return;
    FT_Done_Face(m_face);
    m_face = nullptr;
}

#define TEXMAP_DIM 1024

static unsigned RoundUpPow2(unsigned v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

using GreyPixel = uint8_t;
static void MemcpyRect(GreyPixel* img, const FT_Bitmap* bmp, unsigned slice, unsigned x, unsigned y)
{
    unsigned sy = TEXMAP_DIM * slice + y;
    for (unsigned i=0 ; i<bmp->rows ; ++i)
    {
        const unsigned char* s = &bmp->buffer[bmp->pitch*i];
        GreyPixel* t = &img[TEXMAP_DIM*(sy+i)+x];
        memcpy(t, s, bmp->width);
    }
}

union RgbaPixel
{
    uint8_t rgba[4];
    uint32_t pixel;
};
static void MemcpyRect(RgbaPixel* img, const FT_Bitmap* bmp, unsigned slice, unsigned x, unsigned y)
{
    unsigned sy = TEXMAP_DIM * slice + y;
    for (unsigned i=0 ; i<bmp->rows ; ++i)
    {
        const unsigned char* s = &bmp->buffer[bmp->pitch*i];
        RgbaPixel* t = &img[TEXMAP_DIM*(sy+i)+x];
        for (unsigned j=0 ; j<bmp->width/3 ; ++j)
        {
            t[j].rgba[0] = s[j*3];
            t[j].rgba[1] = s[j*3+1];
            t[j].rgba[2] = s[j*3+2];
            t[j].rgba[3] = 0xff;
        }
    }
}


static inline void GridFitGlyph(FT_GlyphSlot slot, FT_UInt& width, FT_UInt& height)
{
    width = slot->metrics.width >> 6;
    height = slot->metrics.height >> 6;
}

void FontAtlas::buildKernTable(FT_Face face)
{
    if (face->driver->clazz == &tt_driver_class)
    {
        TT_Face ttface = reinterpret_cast<TT_Face>(face);
        if (!ttface->kern_table)
            return;
        athena::io::MemoryReader r(ttface->kern_table, ttface->kern_table_size);
        auto it = m_kernAdjs.end();
        atUint32 nSubs = r.readUint32Big();
        for (atUint32 i=0 ; i<nSubs ; ++i)
        {
            TT_KernHead kernHead;
            kernHead.read(r);
            if (kernHead.coverage >> 8 != 0)
            {
                r.seek(kernHead.length - 6, athena::Current);
                continue;
            }

            TT_KernSubHead subHead;
            subHead.read(r);

            for (atUint16 p=0 ; p<subHead.nPairs ; ++p)
            {
                TT_KernPair pair;
                pair.read(r);
                if (it == m_kernAdjs.end() || it->first != pair.left)
                    if ((it = m_kernAdjs.find(pair.left)) == m_kernAdjs.end())
                        it = m_kernAdjs.insert(std::make_pair(pair.left, std::vector<std::pair<atUint16, atInt16>>())).first;
                it->second.emplace_back(pair.right, pair.value);
            }
        }
    }
}

#define NO_ZLIB 0
#define ZLIB_BUF_SZ 32768

static void WriteCompressed(athena::io::FileWriter& writer, const atUint8* data, size_t sz)
{
#if NO_ZLIB
    writer.writeUBytes(data, sz);
    return;
#endif

    atUint8 compBuf[ZLIB_BUF_SZ];
    z_stream z = {};
    deflateInit(&z, Z_DEFAULT_COMPRESSION);
    z.next_in = (Bytef*)data;
    z.avail_in = sz;
    writer.writeUint32Big(sz);
    atUint64 adlerPos = writer.position();
    writer.writeUint32Big(0); /* Space for adler32 */
    while (z.avail_in)
    {
        z.next_out = compBuf;
        z.avail_out = ZLIB_BUF_SZ;
        deflate(&z, Z_NO_FLUSH);
        writer.writeUBytes(compBuf, ZLIB_BUF_SZ - z.avail_out);
    }

    int finishCycle = Z_OK;
    while (finishCycle != Z_STREAM_END)
    {
        z.next_out = compBuf;
        z.avail_out = ZLIB_BUF_SZ;
        finishCycle = deflate(&z, Z_FINISH);
        writer.writeUBytes(compBuf, ZLIB_BUF_SZ - z.avail_out);
    }

    writer.seek(adlerPos, athena::Begin);
    writer.writeUint32Big(z.adler);

    deflateEnd(&z);
}

static bool ReadDecompressed(athena::io::FileReader& reader, atUint8* data, size_t sz)
{
#if NO_ZLIB
    reader.readUBytesToBuf(data, sz);
    return true;
#endif

    atUint8 compBuf[ZLIB_BUF_SZ];
    z_stream z = {};
    inflateInit(&z);
    z.next_out = data;
    atUint32 targetSz = reader.readUint32Big();
    atUint32 adler32 = reader.readUint32Big();
    z.avail_out = std::min(sz, size_t(targetSz));
    size_t readSz;
    while ((readSz = reader.readUBytesToBuf(compBuf, ZLIB_BUF_SZ)))
    {
        z.next_in = compBuf;
        z.avail_in = readSz;
        if (inflate(&z, Z_NO_FLUSH) == Z_STREAM_END)
            break;
    }

    inflateEnd(&z);
    return adler32 == z.adler;
}

FontAtlas::FontAtlas(boo::IGraphicsDataFactory* gf, FT_Face face, uint32_t dpi,
                     bool subpixel, FCharFilter& filter, athena::io::FileWriter& writer)
: m_dpi(dpi),
  m_ftXscale(face->size->metrics.x_scale),
  m_ftXPpem(face->size->metrics.x_ppem),
  m_lineHeight(face->size->metrics.height),
  m_subpixel(subpixel)
{
    FT_Int32 baseFlags = FT_LOAD_NO_BITMAP;
    if (subpixel)
        baseFlags |= FT_LOAD_TARGET_LCD;
    else
        baseFlags |= FT_LOAD_TARGET_NORMAL;

    /* First count glyphs exposed by unicode charmap and tally required area */
    size_t glyphCount = 0;
    FT_UInt gindex;
    FT_ULong charcode = FT_Get_First_Char(face, &gindex);
    unsigned curLineWidth = 1;
    unsigned curLineHeight = 0;
    unsigned totalHeight = 1;
    unsigned fullTexmapLayers = 0;
    while (gindex != 0)
    {
        if (!filter.second(charcode))
        {
            charcode = FT_Get_Next_Char(face, charcode, &gindex);
            continue;
        }
        ++glyphCount;
        FT_Load_Glyph(face, gindex, baseFlags);
        FT_UInt width, height;
        GridFitGlyph(face->glyph, width, height);
        if (curLineWidth + width + 1 > TEXMAP_DIM)
        {
            totalHeight += curLineHeight + 1;
            curLineHeight = 0;
            curLineWidth = 1;
        }
        curLineHeight = std::max(curLineHeight, height);
        if (totalHeight + curLineHeight + 1 > TEXMAP_DIM)
        {
            totalHeight = 1;
            ++fullTexmapLayers;
            //printf("StagedB: %u\n", gindex);
            curLineHeight = 0;
            curLineWidth = 1;
        }
        curLineWidth += width + 1;
        charcode = FT_Get_Next_Char(face, charcode, &gindex);
    }
    if (curLineHeight)
        totalHeight += curLineHeight + 1;
    m_glyphs.reserve(glyphCount);
    m_glyphLookup.reserve(glyphCount);

    totalHeight = RoundUpPow2(totalHeight);
    unsigned finalHeight = fullTexmapLayers ? TEXMAP_DIM : totalHeight;
    writer.writeUint32Big(fullTexmapLayers + 1);
    writer.writeUint32Big(TEXMAP_DIM);
    writer.writeUint32Big(finalHeight);

    if (subpixel)
    {
        /* Allocate texmap */
        std::unique_ptr<RgbaPixel[]> texmap;
        size_t bufSz;
        if (fullTexmapLayers)
        {
            //printf("ALLOC: %u\n", fullTexmapLayers + 1);
            size_t count = TEXMAP_DIM * TEXMAP_DIM * (fullTexmapLayers + 1);
            texmap.reset(new RgbaPixel[count]);
            bufSz = count * sizeof(RgbaPixel);
            memset(texmap.get(), 0, bufSz);
        }
        else
        {
            size_t count = TEXMAP_DIM * totalHeight;
            texmap.reset(new RgbaPixel[count]);
            bufSz = count * sizeof(RgbaPixel);
            memset(texmap.get(), 0, bufSz);
        }

        /* Assemble glyph texmaps and internal data structures */
        charcode = FT_Get_First_Char(face, &gindex);
        curLineWidth = 1;
        curLineHeight = 0;
        totalHeight = 1;
        fullTexmapLayers = 0;
        while (gindex != 0)
        {
            if (!filter.second(charcode))
            {
                charcode = FT_Get_Next_Char(face, charcode, &gindex);
                continue;
            }
            FT_Load_Glyph(face, gindex, FT_LOAD_RENDER | baseFlags);
            FT_UInt width, height;
            GridFitGlyph(face->glyph, width, height);
            m_glyphLookup[charcode] = m_glyphs.size();
            m_glyphs.emplace_back();
            Glyph& g = m_glyphs.back();

            if (curLineWidth + width + 1 > TEXMAP_DIM)
            {
                totalHeight += curLineHeight + 1;
                curLineHeight = 0;
                curLineWidth = 1;
            }
            curLineHeight = std::max(curLineHeight, height);
            if (totalHeight + curLineHeight + 1 > TEXMAP_DIM)
            {
                totalHeight = 1;
                ++fullTexmapLayers;
                //printf("RealB: %u\n", gindex);
                curLineHeight = 0;
                curLineWidth = 1;
            }

            g.m_unicodePoint = charcode;
            g.m_glyphIdx = gindex;
            g.m_layerIdx = fullTexmapLayers;
            g.m_layerFloat = float(g.m_layerIdx);
            g.m_width = face->glyph->bitmap.width / 3;
            g.m_height = face->glyph->bitmap.rows;
            g.m_uv[0] = curLineWidth / float(TEXMAP_DIM);
            g.m_uv[1] = totalHeight / float(finalHeight);
            g.m_uv[2] = g.m_uv[0] + g.m_width / float(TEXMAP_DIM);
            g.m_uv[3] = g.m_uv[1] + g.m_height / float(finalHeight);
            g.m_leftPadding = face->glyph->metrics.horiBearingX >> 6;
            g.m_advance = face->glyph->advance.x >> 6;
            g.m_verticalOffset = (face->glyph->metrics.horiBearingY - face->glyph->metrics.height) >> 6;
            MemcpyRect(texmap.get(), &face->glyph->bitmap, fullTexmapLayers, curLineWidth, totalHeight);
            curLineWidth += width + 1;
            charcode = FT_Get_Next_Char(face, charcode, &gindex);
        }

        WriteCompressed(writer, (atUint8*)texmap.get(), bufSz);

        gf->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
        {
            m_tex =
            ctx.newStaticArrayTexture(TEXMAP_DIM, finalHeight, fullTexmapLayers + 1, 1,
                                      boo::TextureFormat::RGBA8, boo::TextureClampMode::Repeat, texmap.get(), bufSz);
            return true;
        });
    }
    else
    {
        /* Allocate texmap */
        std::unique_ptr<GreyPixel[]> texmap;
        size_t bufSz;
        if (fullTexmapLayers)
        {
            //printf("ALLOC: %u\n", fullTexmapLayers + 1);
            size_t count = TEXMAP_DIM * TEXMAP_DIM * (fullTexmapLayers + 1);
            texmap.reset(new GreyPixel[count]);
            bufSz = count * sizeof(GreyPixel);
            memset(texmap.get(), 0, bufSz);
        }
        else
        {
            size_t count = TEXMAP_DIM * totalHeight;
            texmap.reset(new GreyPixel[count]);
            bufSz = count * sizeof(GreyPixel);
            memset(texmap.get(), 0, bufSz);
        }

        /* Assemble glyph texmaps and internal data structures */
        charcode = FT_Get_First_Char(face, &gindex);
        curLineWidth = 1;
        curLineHeight = 0;
        totalHeight = 1;
        fullTexmapLayers = 0;
        while (gindex != 0)
        {
            if (!filter.second(charcode))
            {
                charcode = FT_Get_Next_Char(face, charcode, &gindex);
                continue;
            }
            FT_Load_Glyph(face, gindex, FT_LOAD_RENDER | baseFlags);
            FT_UInt width, height;
            GridFitGlyph(face->glyph, width, height);
            m_glyphLookup[charcode] = m_glyphs.size();
            m_glyphs.emplace_back();
            Glyph& g = m_glyphs.back();

            if (curLineWidth + width + 1 > TEXMAP_DIM)
            {
                totalHeight += curLineHeight + 1;
                curLineHeight = 0;
                curLineWidth = 1;
            }
            curLineHeight = std::max(curLineHeight, height);
            if (totalHeight + curLineHeight + 1 > TEXMAP_DIM)
            {
                totalHeight = 1;
                ++fullTexmapLayers;
                //printf("RealB: %u\n", gindex);
                curLineHeight = 0;
                curLineWidth = 1;
            }

            g.m_unicodePoint = charcode;
            g.m_glyphIdx = gindex;
            g.m_layerIdx = fullTexmapLayers;
            g.m_layerFloat = float(g.m_layerIdx);
            g.m_width = face->glyph->bitmap.width;
            g.m_height = face->glyph->bitmap.rows;
            g.m_uv[0] = curLineWidth / float(TEXMAP_DIM);
            g.m_uv[1] = totalHeight / float(finalHeight);
            g.m_uv[2] = g.m_uv[0] + g.m_width / float(TEXMAP_DIM);
            g.m_uv[3] = g.m_uv[1] + g.m_height / float(finalHeight);
            g.m_leftPadding = face->glyph->metrics.horiBearingX >> 6;
            g.m_advance = face->glyph->advance.x >> 6;
            g.m_verticalOffset = (face->glyph->metrics.horiBearingY - face->glyph->metrics.height) >> 6;
            MemcpyRect(texmap.get(), &face->glyph->bitmap, fullTexmapLayers, curLineWidth, totalHeight);
            curLineWidth += width + 1;
            charcode = FT_Get_Next_Char(face, charcode, &gindex);
        }

        WriteCompressed(writer, (atUint8*)texmap.get(), bufSz);

        gf->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
        {
            m_tex =
            ctx.newStaticArrayTexture(TEXMAP_DIM, finalHeight, fullTexmapLayers + 1, 1,
                                      boo::TextureFormat::I8, boo::TextureClampMode::Repeat, texmap.get(), bufSz);
            return true;
        });
    }

    buildKernTable(face);
}

FontAtlas::FontAtlas(boo::IGraphicsDataFactory* gf, FT_Face face, uint32_t dpi,
                     bool subpixel, FCharFilter& filter, athena::io::FileReader& reader)
: m_dpi(dpi),
  m_ftXscale(face->size->metrics.x_scale),
  m_ftXPpem(face->size->metrics.x_ppem),
  m_lineHeight(face->size->metrics.height),
  m_subpixel(subpixel)
{
    FT_Int32 baseFlags = FT_LOAD_NO_BITMAP;
    if (subpixel)
        baseFlags |= FT_LOAD_TARGET_LCD;
    else
        baseFlags |= FT_LOAD_TARGET_NORMAL;

    /* First count glyphs exposed by unicode charmap */
    size_t glyphCount = 0;
    FT_UInt gindex;
    FT_ULong charcode = FT_Get_First_Char(face, &gindex);
    while (gindex != 0)
    {
        if (!filter.second(charcode))
        {
            charcode = FT_Get_Next_Char(face, charcode, &gindex);
            continue;
        }
        ++glyphCount;
        charcode = FT_Get_Next_Char(face, charcode, &gindex);
    }
    m_glyphs.reserve(glyphCount);
    m_glyphLookup.reserve(glyphCount);

    unsigned fullTexmapLayers = reader.readUint32Big() - 1;
    reader.readUint32Big();
    unsigned finalHeight = reader.readUint32Big();

    if (subpixel)
    {
        /* Allocate texmap */
        std::unique_ptr<RgbaPixel[]> texmap;
        size_t bufSz;
        if (fullTexmapLayers)
        {
            //printf("ALLOC: %u\n", fullTexmapLayers + 1);
            size_t count = TEXMAP_DIM * TEXMAP_DIM * (fullTexmapLayers + 1);
            texmap.reset(new RgbaPixel[count]);
            bufSz = count * sizeof(RgbaPixel);
            memset(texmap.get(), 0, bufSz);
        }
        else
        {
            size_t count = TEXMAP_DIM * finalHeight;
            texmap.reset(new RgbaPixel[TEXMAP_DIM * finalHeight]);
            bufSz = count * sizeof(RgbaPixel);
            memset(texmap.get(), 0, bufSz);
        }

        /* Assemble glyph texmaps and internal data structures */
        charcode = FT_Get_First_Char(face, &gindex);
        unsigned curLineWidth = 1;
        unsigned curLineHeight = 0;
        unsigned totalHeight = 1;
        fullTexmapLayers = 0;
        while (gindex != 0)
        {
            if (!filter.second(charcode))
            {
                charcode = FT_Get_Next_Char(face, charcode, &gindex);
                continue;
            }
            FT_Load_Glyph(face, gindex, baseFlags);
            FT_UInt width, height;
            GridFitGlyph(face->glyph, width, height);
            m_glyphLookup[charcode] = m_glyphs.size();
            m_glyphs.emplace_back();
            Glyph& g = m_glyphs.back();

            if (curLineWidth + width + 1 > TEXMAP_DIM)
            {
                totalHeight += curLineHeight + 1;
                curLineHeight = 0;
                curLineWidth = 1;
            }
            curLineHeight = std::max(curLineHeight, height);
            if (totalHeight + curLineHeight + 1 > TEXMAP_DIM)
            {
                totalHeight = 1;
                ++fullTexmapLayers;
                //printf("RealB: %u\n", gindex);
                curLineHeight = 0;
                curLineWidth = 1;
            }

            g.m_unicodePoint = charcode;
            g.m_glyphIdx = gindex;
            g.m_layerIdx = fullTexmapLayers;
            g.m_layerFloat = float(g.m_layerIdx);
            g.m_width = width;
            g.m_height = height;
            g.m_uv[0] = curLineWidth / float(TEXMAP_DIM);
            g.m_uv[1] = totalHeight / float(finalHeight);
            g.m_uv[2] = g.m_uv[0] + g.m_width / float(TEXMAP_DIM);
            g.m_uv[3] = g.m_uv[1] + g.m_height / float(finalHeight);
            g.m_leftPadding = face->glyph->metrics.horiBearingX >> 6;
            g.m_advance = face->glyph->advance.x >> 6;
            g.m_verticalOffset = (face->glyph->metrics.horiBearingY - face->glyph->metrics.height) >> 6;
            curLineWidth += width + 1;
            charcode = FT_Get_Next_Char(face, charcode, &gindex);
        }

        if (!ReadDecompressed(reader, (atUint8*)texmap.get(), bufSz))
            return;

        gf->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
        {
            m_tex =
            ctx.newStaticArrayTexture(TEXMAP_DIM, finalHeight, fullTexmapLayers + 1, 1,
                                      boo::TextureFormat::RGBA8, boo::TextureClampMode::Repeat, texmap.get(), bufSz);
            return false;
        });
    }
    else
    {
        /* Allocate texmap */
        std::unique_ptr<GreyPixel[]> texmap;
        size_t bufSz;
        if (fullTexmapLayers)
        {
            //printf("ALLOC: %u\n", fullTexmapLayers + 1);
            size_t count = TEXMAP_DIM * TEXMAP_DIM * (fullTexmapLayers + 1);
            texmap.reset(new GreyPixel[count]);
            bufSz = count * sizeof(GreyPixel);
            memset(texmap.get(), 0, bufSz);
        }
        else
        {
            size_t count = TEXMAP_DIM * finalHeight;
            texmap.reset(new GreyPixel[TEXMAP_DIM * finalHeight]);
            bufSz = count * sizeof(GreyPixel);
            memset(texmap.get(), 0, bufSz);
        }

        /* Assemble glyph texmaps and internal data structures */
        charcode = FT_Get_First_Char(face, &gindex);
        unsigned curLineWidth = 1;
        unsigned curLineHeight = 0;
        unsigned totalHeight = 1;
        fullTexmapLayers = 0;
        while (gindex != 0)
        {
            if (!filter.second(charcode))
            {
                charcode = FT_Get_Next_Char(face, charcode, &gindex);
                continue;
            }
            FT_Load_Glyph(face, gindex, baseFlags);
            FT_UInt width, height;
            GridFitGlyph(face->glyph, width, height);
            m_glyphLookup[charcode] = m_glyphs.size();
            m_glyphs.emplace_back();
            Glyph& g = m_glyphs.back();

            if (curLineWidth + width + 1 > TEXMAP_DIM)
            {
                totalHeight += curLineHeight + 1;
                curLineHeight = 0;
                curLineWidth = 1;
            }
            curLineHeight = std::max(curLineHeight, height);
            if (totalHeight + curLineHeight + 1 > TEXMAP_DIM)
            {
                totalHeight = 1;
                ++fullTexmapLayers;
                //printf("RealB: %u\n", gindex);
                curLineHeight = 0;
                curLineWidth = 1;
            }

            g.m_unicodePoint = charcode;
            g.m_glyphIdx = gindex;
            g.m_layerIdx = fullTexmapLayers;
            g.m_layerFloat = float(g.m_layerIdx);
            g.m_width = width;
            g.m_height = height;
            g.m_uv[0] = curLineWidth / float(TEXMAP_DIM);
            g.m_uv[1] = totalHeight / float(finalHeight);
            g.m_uv[2] = g.m_uv[0] + g.m_width / float(TEXMAP_DIM);
            g.m_uv[3] = g.m_uv[1] + g.m_height / float(finalHeight);
            g.m_leftPadding = face->glyph->metrics.horiBearingX >> 6;
            g.m_advance = face->glyph->advance.x >> 6;
            g.m_verticalOffset = (face->glyph->metrics.horiBearingY - face->glyph->metrics.height) >> 6;
            curLineWidth += width + 1;
            charcode = FT_Get_Next_Char(face, charcode, &gindex);
        }

        if (!ReadDecompressed(reader, (atUint8*)texmap.get(), bufSz))
            return;

        gf->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
        {
            m_tex =
            ctx.newStaticArrayTexture(TEXMAP_DIM, finalHeight, fullTexmapLayers + 1, 1,
                                      boo::TextureFormat::I8, boo::TextureClampMode::Repeat, texmap.get(), bufSz);
            return true;
        });
    }

    buildKernTable(face);
}

FontCache::Library::Library()
{
    FT_Error err = FT_Init_FreeType(&m_lib);
    if (err)
        Log.report(logvisor::Fatal, "unable to FT_Init_FreeType");
}

FontCache::Library::~Library()
{
    FT_Done_FreeType(m_lib);
}

FontCache::FontCache(const hecl::Runtime::FileStoreManager& fileMgr)
: m_fileMgr(fileMgr),
  m_cacheRoot(hecl::SystemString(m_fileMgr.getStoreRoot()) + _S("/fontcache")),
  m_regFace(m_fontLib, DROIDSANS_PERMISSIVE, DROIDSANS_PERMISSIVE_SZ),
  m_monoFace(m_fontLib, BMONOFONT, BMONOFONT_SZ),
  m_curvesFace(m_fontLib, SPECTERCURVES, SPECTERCURVES_SZ)
{
    hecl::MakeDir(m_cacheRoot.c_str());
}

FontTag FontCache::prepCustomFont(boo::IGraphicsDataFactory* gf, std::string_view name, FT_Face face,
                                  FCharFilter filter, bool subpixel,
                                  float points, uint32_t dpi)
{
    /* Quick validation */
    if (!face)
        Log.report(logvisor::Fatal, "invalid freetype face");

    if (!face->charmap || face->charmap->encoding != FT_ENCODING_UNICODE)
        Log.report(logvisor::Fatal, "font does not contain a unicode char map");

    /* Set size with FreeType */
    FT_Set_Char_Size(face, 0, points * 64.0, 0, dpi);

    /* Make tag and search for cached version */
    FontTag tag(std::string(name) + '_' + filter.first, subpixel, points, dpi);
    auto search = m_cachedAtlases.find(tag);
    if (search != m_cachedAtlases.end())
        return tag;

    /* Now check filesystem cache */
    hecl::SystemString cachePath = m_cacheRoot + _S('/') + hecl::SysFormat(_S("%" PRIx64), tag.hash());
    hecl::Sstat st;
    if (!hecl::Stat(cachePath.c_str(), &st) && S_ISREG(st.st_mode))
    {
        athena::io::FileReader r(cachePath);
        if (!r.hasError())
        {
            atUint32 magic = r.readUint32Big();
            if (r.position() == 4 && magic == 'FONT')
            {
                std::unique_ptr<FontAtlas> fa = std::make_unique<FontAtlas>(gf, face, dpi, subpixel, filter, r);
                if (fa->m_tex)
                {
                    m_cachedAtlases.emplace(tag, std::move(fa));
                    return tag;
                }
            }
        }
    }

    /* Nada, build and cache now */
    athena::io::FileWriter w(cachePath);
    if (w.hasError())
        Log.report(logvisor::Fatal, "unable to open '%s' for writing", cachePath.c_str());
    w.writeUint32Big('FONT');
    m_cachedAtlases.emplace(tag, std::make_unique<FontAtlas>(gf, face, dpi, subpixel, filter, w));
    return tag;
}

const FontAtlas& FontCache::lookupAtlas(FontTag tag) const
{
    auto search = m_cachedAtlases.find(tag);
    if (search == m_cachedAtlases.cend())
        Log.report(logvisor::Fatal, "invalid font");
    return *search->second.get();
}

}
