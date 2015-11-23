#include "Specter/FontCache.hpp"
#include <LogVisor/LogVisor.hpp>
#include <stdint.h>

#include FT_GZIP_H
#include FT_SYSTEM_H
#include <freetype/internal/internal.h>
#include <freetype/internal/ftstream.h>

extern "C" const uint8_t DROIDSANS_PERMISSIVE[];
extern "C" size_t DROIDSANS_PERMISSIVE_SZ;

extern "C" const uint8_t BMONOFONT[];
extern "C" size_t BMONOFONT_SZ;

namespace Specter
{
static LogVisor::LogModule Log("Specter::FontCache");

FontTag::FontTag(const std::string& name, bool subpixel, float points, uint32_t dpi)
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
{
    m_comp.base = (unsigned char*)data;
    m_comp.size = sz;
    m_comp.memory = lib->memory;
    if (FT_Stream_OpenGzip(&m_decomp, &m_comp))
        Log.report(LogVisor::FatalError, "unable to open FreeType gzip stream");

    FT_Open_Args args =
    {
        FT_OPEN_STREAM,
        nullptr,
        0,
        nullptr,
        &m_decomp
    };

    if (FT_Open_Face(lib, &args, 0, &m_face))
        Log.report(LogVisor::FatalError, "unable to open FreeType gzip face");
}

FreeTypeGZipMemFace::~FreeTypeGZipMemFace()
{
    FT_Done_Face(m_face);
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
        GreyPixel* t = &img[TEXMAP_DIM*sy+x];
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
        RgbaPixel* t = &img[TEXMAP_DIM*sy+x];
        for (unsigned j=0 ; j<bmp->width/3 ; ++j)
        {
            t[j].rgba[0] = s[j*3];
            t[j].rgba[1] = s[j*3+1];
            t[j].rgba[2] = s[j*3+2];
            t[j].rgba[3] = 0xff;
        }
    }
}


FontAtlas::FontAtlas(boo::IGraphicsDataFactory* gf, FT_Face face, bool subpixel, Athena::io::FileWriter& writer)
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
    unsigned curLineWidth = 0;
    unsigned curLineHeight = 0;
    unsigned totalHeight = 0;
    unsigned fullTexmap = 0;
    while (gindex != 0)
    {
        ++glyphCount;
        FT_Load_Glyph(face, gindex, baseFlags);
        unsigned width = face->glyph->bitmap.width;
        if (subpixel)
            width /= 3;
        if (curLineWidth + width > TEXMAP_DIM)
        {
            if (totalHeight + curLineHeight > TEXMAP_DIM)
            {
                totalHeight = 0;
                ++fullTexmap;
            }
            else
                totalHeight += curLineHeight;
            curLineHeight = 0;
            curLineWidth = 0;
        }
        curLineHeight = std::max(curLineHeight, face->glyph->bitmap.rows);
        charcode = FT_Get_Next_Char(face, charcode, &gindex);
    }
    m_glyphs.reserve(glyphCount);

    totalHeight = RoundUpPow2(totalHeight);
    unsigned finalHeight = fullTexmap ? TEXMAP_DIM : totalHeight;
    writer.writeUint32Big(fullTexmap);
    writer.writeUint32Big(TEXMAP_DIM);
    writer.writeUint32Big(finalHeight);

    if (subpixel)
    {
        /* Allocate texmap */
        std::unique_ptr<RgbaPixel[]> texmap;
        size_t bufSz;
        if (fullTexmap)
        {
            size_t count = TEXMAP_DIM * TEXMAP_DIM * (fullTexmap + 1);
            texmap.reset(new RgbaPixel[count]);
            bufSz = count * sizeof(RgbaPixel);
            memset(texmap.get(), 0, bufSz);
        }
        else
        {
            size_t count = TEXMAP_DIM * totalHeight;
            texmap.reset(new RgbaPixel[TEXMAP_DIM * totalHeight]);
            bufSz = count * sizeof(RgbaPixel);
            memset(texmap.get(), 0, bufSz);
        }

        /* Assemble glyph texmaps and internal data structures */
        charcode = FT_Get_First_Char(face, &gindex);
        curLineWidth = 0;
        curLineHeight = 0;
        totalHeight = 0;
        fullTexmap = 0;
        while (gindex != 0)
        {
            FT_Load_Glyph(face, gindex, FT_LOAD_RENDER | baseFlags);
            m_glyphs.emplace_back();
            Glyph& g = m_glyphs.back();
            g.m_unicodePoint = charcode;
            g.m_layerIdx = fullTexmap;
            g.m_width = face->glyph->bitmap.width / 3;
            g.m_height = face->glyph->bitmap.rows;
            g.m_uv[0] = curLineWidth / float(TEXMAP_DIM);
            g.m_uv[1] = totalHeight / float(finalHeight);
            g.m_uv[2] = g.m_uv[0] + g.m_width / float(TEXMAP_DIM);
            g.m_uv[3] = g.m_uv[1] + g.m_height / float(finalHeight);
            g.m_leftPadding = 0;
            g.m_advance = face->glyph->advance.x;
            g.m_rightPadding = 0;
            g.m_verticalOffset = face->glyph->metrics.horiBearingY;
            g.m_kernIdx = 0;
            MemcpyRect(texmap.get(), &face->glyph->bitmap, fullTexmap, curLineWidth, totalHeight);
            if (curLineWidth + g.m_width > TEXMAP_DIM)
            {
                if (totalHeight + curLineHeight > TEXMAP_DIM)
                {
                    totalHeight = 0;
                    ++fullTexmap;
                }
                else
                    totalHeight += curLineHeight;
                curLineHeight = 0;
                curLineWidth = 0;
            }
            curLineHeight = std::max(curLineHeight, face->glyph->bitmap.rows);
            charcode = FT_Get_Next_Char(face, charcode, &gindex);
        }

        m_tex =
        gf->newStaticArrayTexture(TEXMAP_DIM, finalHeight, fullTexmap + 1,
                                  boo::TextureFormat::RGBA8, texmap.get(), bufSz);
    }
    else
    {
        /* Allocate texmap */
        std::unique_ptr<GreyPixel[]> texmap;
        size_t bufSz;
        if (fullTexmap)
        {
            size_t count = TEXMAP_DIM * TEXMAP_DIM * (fullTexmap + 1);
            texmap.reset(new GreyPixel[count]);
            bufSz = count * sizeof(GreyPixel);
            memset(texmap.get(), 0, bufSz);
        }
        else
        {
            size_t count = TEXMAP_DIM * totalHeight;
            texmap.reset(new GreyPixel[TEXMAP_DIM * totalHeight]);
            bufSz = count * sizeof(GreyPixel);
            memset(texmap.get(), 0, bufSz);
        }

        /* Assemble glyph texmaps and internal data structures */
        charcode = FT_Get_First_Char(face, &gindex);
        curLineWidth = 0;
        curLineHeight = 0;
        totalHeight = 0;
        fullTexmap = 0;
        while (gindex != 0)
        {
            FT_Load_Glyph(face, gindex, FT_LOAD_RENDER | baseFlags);
            m_glyphs.emplace_back();
            Glyph& g = m_glyphs.back();
            g.m_unicodePoint = charcode;
            g.m_layerIdx = fullTexmap;
            g.m_width = face->glyph->bitmap.width;
            g.m_height = face->glyph->bitmap.rows;
            g.m_uv[0] = curLineWidth / float(TEXMAP_DIM);
            g.m_uv[1] = totalHeight / float(finalHeight);
            g.m_uv[2] = g.m_uv[0] + g.m_width / float(TEXMAP_DIM);
            g.m_uv[3] = g.m_uv[1] + g.m_height / float(finalHeight);
            g.m_leftPadding = 0;
            g.m_advance = face->glyph->advance.x;
            g.m_rightPadding = 0;
            g.m_verticalOffset = face->glyph->metrics.horiBearingY;
            g.m_kernIdx = 0;
            MemcpyRect(texmap.get(), &face->glyph->bitmap, fullTexmap, curLineWidth, totalHeight);
            if (curLineWidth + g.m_width > TEXMAP_DIM)
            {
                if (totalHeight + curLineHeight > TEXMAP_DIM)
                {
                    totalHeight = 0;
                    ++fullTexmap;
                }
                else
                    totalHeight += curLineHeight;
                curLineHeight = 0;
                curLineWidth = 0;
            }
            curLineHeight = std::max(curLineHeight, face->glyph->bitmap.rows);
            charcode = FT_Get_Next_Char(face, charcode, &gindex);
        }

        m_tex =
        gf->newStaticArrayTexture(TEXMAP_DIM, finalHeight, fullTexmap + 1,
                                  boo::TextureFormat::I8, texmap.get(), bufSz);
    }
}

FontAtlas::FontAtlas(boo::IGraphicsDataFactory* gf, FT_Face face, bool subpixel, Athena::io::FileReader& reader)
{
}

FontCache::Library::Library()
{
    FT_Error err = FT_Init_FreeType(&m_lib);
    if (err)
        Log.report(LogVisor::FatalError, "unable to FT_Init_FreeType");
}

FontCache::Library::~Library()
{
    FT_Done_FreeType(m_lib);
}

static FT_Library InitLib()
{
    FT_Library ret;
    FT_Error err = FT_Init_FreeType(&ret);
    if (err)
        Log.report(LogVisor::FatalError, "unable to FT_Init_FreeType");
    return ret;
}

FontCache::FontCache(const HECL::Runtime::FileStoreManager& fileMgr)
: m_fileMgr(fileMgr),
  m_cacheRoot(m_fileMgr.getStoreRoot() + _S("/fontcache")),
  m_regFace(m_fontLib, DROIDSANS_PERMISSIVE, DROIDSANS_PERMISSIVE_SZ),
  m_monoFace(m_fontLib, BMONOFONT, BMONOFONT_SZ)
{HECL::MakeDir(m_cacheRoot.c_str());}

FontTag FontCache::prepCustomFont(boo::IGraphicsDataFactory* gf,
                                  const std::string& name, FT_Face face, bool subpixel,
                                  float points, uint32_t dpi)
{
    /* Quick validation */
    if (!face)
        Log.report(LogVisor::FatalError, "invalid freetype face");

    if (!face->charmap || face->charmap->encoding != ft_encoding_unicode)
        Log.report(LogVisor::FatalError, "font does not contain a unicode char map");

    /* Set size with FreeType */
    FT_Set_Char_Size(face, points * 64.0, 0, dpi, 0);

    /* Make tag and search for cached version */
    FontTag tag(name, subpixel, points, dpi);
    auto search = m_cachedAtlases.find(tag);
    if (search != m_cachedAtlases.end())
        return tag;

    /* Now check filesystem cache */
    HECL::SystemString cachePath = m_cacheRoot + _S('/') + HECL::Format("%" PRIx64, tag.hash());
#if 0
    HECL::Sstat st;
    if (!HECL::Stat(cachePath.c_str(), &st) && S_ISREG(st.st_mode))
    {
        Athena::io::FileReader r(cachePath);
        if (!r.hasError())
        {
            atUint32 magic = r.readUint32Big();
            if (r.position() == 4 && magic == 'FONT')
            {
                m_cachedAtlases.emplace(tag, std::make_unique<FontAtlas>(gf, face, subpixel, r));
                return tag;
            }
        }
    }
#endif

    /* Nada, build and cache now */
    Athena::io::FileWriter w(cachePath);
    if (w.hasError())
        Log.report(LogVisor::FatalError, "unable to open '%s' for writing", cachePath.c_str());
    w.writeUint32Big('FONT');
    m_cachedAtlases.emplace(tag, std::make_unique<FontAtlas>(gf, face, subpixel, w));
    return tag;
}

}
