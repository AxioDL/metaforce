#ifndef SPECTER_FONTCACHE_HPP
#define SPECTER_FONTCACHE_HPP

#include <ft2build.h>
#include FT_FREETYPE_H

#include <boo/boo.hpp>
#include <HECL/Runtime.hpp>
#include <Athena/FileReader.hpp>
#include <Athena/FileWriter.hpp>

namespace Specter
{
class FontTag
{
    friend class FontCache;
    uint64_t m_hash;
    FontTag(const std::string& name, bool subpixel, float points, unsigned dpi);
public:
    uint64_t hash() const {return m_hash;}
    bool operator==(const FontTag& other) const {return m_hash == other.m_hash;}
};
}

namespace std
{
template <> struct hash<Specter::FontTag>
{
    size_t operator() (const Specter::FontTag& handle) const NOEXCEPT
    {return size_t(handle.hash());}
};
}

namespace Specter
{

class FreeTypeGZipMemFace
{
    FT_Library m_lib;
    FT_StreamRec m_comp = {};
    FT_StreamRec m_decomp = {};
    FT_Face m_face = nullptr;
public:
    FreeTypeGZipMemFace(FT_Library lib, const uint8_t* data, size_t sz);
    ~FreeTypeGZipMemFace() {close();}
    void open();
    void close();
    operator FT_Face() {open(); return m_face;}
};

class FontAtlas
{
    friend class FontCache;
    FT_Face m_face;
    boo::ITextureS* m_tex;

    struct Glyph
    {
        atUint32 m_unicodePoint;
        atUint32 m_layerIdx;
        float m_uv[4];
        atInt8 m_leftPadding;
        atInt8 m_advance;
        atInt8 m_rightPadding;
        atUint8 m_width;
        atUint8 m_height;
        atInt8 m_verticalOffset;
        atUint16 m_kernIdx;
    };
    std::vector<Glyph> m_glyphs;
    std::map<atUint32, size_t> m_glyphLookup;

public:
    FontAtlas(boo::IGraphicsDataFactory* gf, FT_Face face, bool subpixel, Athena::io::FileWriter& writer);
    FontAtlas(boo::IGraphicsDataFactory* gf, FT_Face face, bool subpixel, Athena::io::FileReader& reader);
};

class FontCache
{
    const HECL::Runtime::FileStoreManager& m_fileMgr;
    HECL::SystemString m_cacheRoot;
    struct Library
    {
        FT_Library m_lib;
        Library();
        ~Library();
        operator FT_Library() {return m_lib;}
    } m_fontLib;
    FreeTypeGZipMemFace m_regFace;
    FreeTypeGZipMemFace m_monoFace;

    std::unordered_map<FontTag, std::unique_ptr<FontAtlas>> m_cachedAtlases;
public:
    FontCache(const HECL::Runtime::FileStoreManager& fileMgr);

    FontTag prepCustomFont(boo::IGraphicsDataFactory* gf,
                           const std::string& name, FT_Face face, bool subpixel=false,
                           float points=10.0, uint32_t dpi=72);

    FontTag prepMainFont(boo::IGraphicsDataFactory* gf,
                         bool subpixel=false, float points=10.0, uint32_t dpi=72)
    {return prepCustomFont(gf, "droidsans-permissive", m_regFace, subpixel, points, dpi);}

    FontTag prepMonoFont(boo::IGraphicsDataFactory* gf,
                         bool subpixel=false, float points=10.0, uint32_t dpi=72)
    {return prepCustomFont(gf, "bmonofont", m_monoFace, subpixel, points, dpi);}

    void closeBuiltinFonts() {m_regFace.close(); m_monoFace.close();}
};

}

#endif // SPECTER_FONTCACHE_HPP
