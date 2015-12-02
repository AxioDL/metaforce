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
    uint64_t m_hash = 0;
    FontTag(const std::string& name, bool subpixel, float points, unsigned dpi);
public:
    FontTag() = default;
    operator bool() const {return m_hash != 0;}
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

using FCharFilter = std::pair<std::string, std::function<bool(uint32_t)>>;

class FontAtlas
{
    friend class FontCache;
    FT_Face m_face;
    boo::ITextureSA* m_tex = nullptr;
    uint32_t m_dpi;
    FT_Fixed m_ftXscale;
    FT_UShort m_ftXPpem;
    FT_Pos m_lineHeight;
    bool m_subpixel;

public:
    struct Glyph
    {
        atUint32 m_unicodePoint;
        atUint32 m_layerIdx;
        float m_layerFloat;
        float m_uv[4];
        atInt32 m_leftPadding;
        atInt32 m_advance;
        atUint32 m_width;
        atUint32 m_height;
        atInt32 m_verticalOffset;
    };

private:
    std::vector<Glyph> m_glyphs;
    std::unordered_map<atUint16, std::vector<std::pair<atUint16, atInt16>>> m_kernAdjs;

    struct TT_KernHead : Athena::io::DNA<Athena::BigEndian>
    {
        DECL_DNA
        Value<atUint32> length;
        Value<atUint16> coverage;
    };

    struct TT_KernSubHead : Athena::io::DNA<Athena::BigEndian>
    {
        DECL_DNA
        Value<atUint16> nPairs;
        Value<atUint16> searchRange;
        Value<atUint16> entrySelector;
        Value<atUint16> rangeShift;
    };

    struct TT_KernPair : Athena::io::DNA<Athena::BigEndian>
    {
        DECL_DNA
        Value<atUint16> left;
        Value<atUint16> right;
        Value<atInt16> value;
    };

    void buildKernTable(FT_Face face);

    std::unordered_map<atUint32, size_t> m_glyphLookup;

public:
    FontAtlas(boo::IGraphicsDataFactory* gf, FT_Face face, uint32_t dpi,
              bool subpixel, FCharFilter& filter, Athena::io::FileWriter& writer);
    FontAtlas(boo::IGraphicsDataFactory* gf, FT_Face face, uint32_t dpi,
              bool subpixel, FCharFilter& filter, Athena::io::FileReader& reader);
    FontAtlas(const FontAtlas& other) = delete;
    FontAtlas& operator=(const FontAtlas& other) = delete;

    uint32_t dpi() const {return m_dpi;}
    FT_Fixed FT_Xscale() const {return m_ftXscale;}
    FT_UShort FT_XPPem() const {return m_ftXPpem;}
    FT_Pos FT_LineHeight() const {return m_lineHeight;}
    boo::ITexture* texture() const {return m_tex;}
    bool subpixel() const {return m_subpixel;}

    const Glyph* lookupGlyph(atUint32 charcode) const
    {
        auto search = m_glyphLookup.find(charcode);
        if (search == m_glyphLookup.end())
            return nullptr;
        return &m_glyphs[search->second];
    }
    atInt16 lookupKern(atUint32 left, atUint32 right) const
    {
        auto leftSearch = m_glyphLookup.find(left);
        if (leftSearch == m_glyphLookup.cend())
            return 0;
        size_t leftIdx = leftSearch->second;
        auto rightSearch = m_glyphLookup.find(right);
        if (rightSearch == m_glyphLookup.cend())
            return 0;
        size_t rightIdx = rightSearch->second;
        auto pairSearch = m_kernAdjs.find(leftIdx);
        if (pairSearch == m_kernAdjs.cend())
            return 0;
        for (const std::pair<atUint16, atInt16>& p : pairSearch->second)
            if (p.first == rightIdx)
                return p.second;
        return 0;
    }
};

static FCharFilter const AllCharFilter =
std::make_pair("all-glyphs", [](uint32_t)->bool
{return true;});

static FCharFilter const LatinCharFilter =
std::make_pair("latin-glyphs", [](uint32_t c)->bool
{return c <= 0xff || ((c - 0x2200) <= (0x23FF - 0x2200));});

static FCharFilter const LatinAndJapaneseCharFilter =
std::make_pair("latin-and-jp-glyphs", [](uint32_t c)->bool
{return LatinCharFilter.second(c) || ((c - 0x2E00) <= (0x30FF - 0x2E00));});

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

    FontTag prepCustomFont(boo::IGraphicsDataFactory* gf, const std::string& name, FT_Face face,
                           FCharFilter filter=AllCharFilter, bool subpixel=false,
                           float points=10.0, uint32_t dpi=72);

    FontTag prepMainFont(boo::IGraphicsDataFactory* gf, FCharFilter filter=AllCharFilter,
                         bool subpixel=false, float points=10.0, uint32_t dpi=72)
    {return prepCustomFont(gf, "droidsans-permissive", m_regFace, filter, subpixel, points, dpi);}

    FontTag prepMonoFont(boo::IGraphicsDataFactory* gf, FCharFilter filter=AllCharFilter,
                         bool subpixel=false, float points=10.0, uint32_t dpi=72)
    {return prepCustomFont(gf, "bmonofont", m_monoFace, filter, subpixel, points, dpi);}

    void closeBuiltinFonts() {m_regFace.close(); m_monoFace.close();}

    const FontAtlas& lookupAtlas(FontTag tag) const;
};

}

#endif // SPECTER_FONTCACHE_HPP
