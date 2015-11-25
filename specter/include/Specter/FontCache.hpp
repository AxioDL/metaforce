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
    FreeTypeGZipMemFace(const FreeTypeGZipMemFace& other) = delete;
    FreeTypeGZipMemFace& operator=(const FreeTypeGZipMemFace& other) = delete;
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
    uint32_t m_dpi;

public:
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
        atInt16 m_kernIdx = -1;
    };

    struct KernAdj
    {
        atUint32 a;
        atUint32 b;
        atInt8 adj;
    };

private:
    std::vector<Glyph> m_glyphs;
    std::vector<KernAdj> m_kernAdjs;
    std::unordered_map<atUint32, size_t> m_glyphLookup;

public:
    FontAtlas(boo::IGraphicsDataFactory* gf, FT_Face face, uint32_t dpi,
              bool subpixel, Athena::io::FileWriter& writer);
    FontAtlas(boo::IGraphicsDataFactory* gf, FT_Face face, uint32_t dpi,
              bool subpixel, Athena::io::FileReader& reader);
    FontAtlas(const FontAtlas& other) = delete;
    FontAtlas& operator=(const FontAtlas& other) = delete;

    uint32_t dpi() const {return m_dpi;}
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
    FontCache(const FontCache& other) = delete;
    FontCache& operator=(const FontCache& other) = delete;

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

    const FontAtlas& lookupAtlas(FontTag tag) const;
};

}

#endif // SPECTER_FONTCACHE_HPP
