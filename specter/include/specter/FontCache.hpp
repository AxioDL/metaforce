#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <boo/boo.hpp>
#include <hecl/Runtime.hpp>
#include <athena/FileReader.hpp>
#include <athena/FileWriter.hpp>
#include <athena/DNA.hpp>

namespace specter
{
class FontTag
{
    friend class FontCache;
    uint64_t m_hash = 0;
    FontTag(std::string_view name, bool subpixel, float points, unsigned dpi);
public:
    FontTag() = default;
    operator bool() const {return m_hash != 0;}
    uint64_t hash() const {return m_hash;}
    bool operator==(const FontTag& other) const {return m_hash == other.m_hash;}
};
}

namespace std
{
template <> struct hash<specter::FontTag>
{
    size_t operator() (const specter::FontTag& handle) const noexcept
    {return size_t(handle.hash());}
};
}

namespace specter
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
    FT_Face m_face;
    std::vector<uint8_t> m_texmap;
    boo::ObjToken<boo::ITextureSA> m_tex;
    uint32_t m_dpi;
    FT_Fixed m_ftXscale;
    FT_UShort m_ftXPpem;
    FT_Pos m_lineHeight;
    unsigned m_finalHeight;
    unsigned m_fullTexmapLayers;
    bool m_subpixel;
    bool m_ready = false;

public:
    struct Glyph
    {
        atUint32 m_unicodePoint;
        atUint32 m_glyphIdx;
        atUint32 m_layerIdx;
        float m_layerFloat;
        float m_uv[4];
        atInt32 m_leftPadding;
        atInt32 m_advance;
        atInt32 m_width;
        atInt32 m_height;
        atInt32 m_verticalOffset;
    };

private:
    std::vector<Glyph> m_glyphs;
    std::unordered_map<atUint16, std::vector<std::pair<atUint16, atInt16>>> m_kernAdjs;

    struct TT_KernHead : athena::io::DNA<athena::Big>
    {
        AT_DECL_DNA
        Value<atUint32> length;
        Value<atUint16> coverage;
    };

    struct TT_KernSubHead : athena::io::DNA<athena::Big>
    {
        AT_DECL_DNA
        Value<atUint16> nPairs;
        Value<atUint16> searchRange;
        Value<atUint16> entrySelector;
        Value<atUint16> rangeShift;
    };

    struct TT_KernPair : athena::io::DNA<athena::Big>
    {
        AT_DECL_DNA
        Value<atUint16> left;
        Value<atUint16> right;
        Value<atInt16> value;
    };

    void buildKernTable(FT_Face face);

    std::unordered_map<atUint32, size_t> m_glyphLookup;

public:
    FontAtlas(FT_Face face, uint32_t dpi,
              bool subpixel, FCharFilter& filter, athena::io::FileWriter& writer);
    FontAtlas(FT_Face face, uint32_t dpi,
              bool subpixel, FCharFilter& filter, athena::io::FileReader& reader);
    FontAtlas(const FontAtlas& other) = delete;
    FontAtlas& operator=(const FontAtlas& other) = delete;

    uint32_t dpi() const {return m_dpi;}
    FT_Fixed FT_Xscale() const {return m_ftXscale;}
    FT_UShort FT_XPPem() const {return m_ftXPpem;}
    FT_Pos FT_LineHeight() const {return m_lineHeight;}
    bool isReady() const { return m_ready; }
    boo::ObjToken<boo::ITextureSA> texture(boo::IGraphicsDataFactory* gf) const;
    bool subpixel() const {return m_subpixel;}

    const Glyph* lookupGlyph(atUint32 charcode) const
    {
        auto search = m_glyphLookup.find(charcode);
        if (search == m_glyphLookup.end())
            return nullptr;
        return &m_glyphs[search->second];
    }
    atInt16 lookupKern(atUint32 leftIdx, atUint32 rightIdx) const
    {
        auto pairSearch = m_kernAdjs.find(leftIdx);
        if (pairSearch == m_kernAdjs.cend())
            return 0;
        for (const std::pair<atUint16, atInt16>& p : pairSearch->second)
            if (p.first == rightIdx)
                return p.second;
        return 0;
    }
};

extern const FCharFilter AllCharFilter;
extern const FCharFilter LatinCharFilter;
extern const FCharFilter LatinAndJapaneseCharFilter;

class FontCache
{
    const hecl::Runtime::FileStoreManager& m_fileMgr;
    hecl::SystemString m_cacheRoot;
    struct Library
    {
        FT_Library m_lib;
        Library();
        ~Library();
        operator FT_Library() {return m_lib;}
    } m_fontLib;
    FreeTypeGZipMemFace m_regFace;
    FreeTypeGZipMemFace m_monoFace;
    FreeTypeGZipMemFace m_curvesFace;

    std::unordered_map<FontTag, std::unique_ptr<FontAtlas>> m_cachedAtlases;
public:
    FontCache(const hecl::Runtime::FileStoreManager& fileMgr);
    FontCache(const FontCache& other) = delete;
    FontCache& operator=(const FontCache& other) = delete;

    FontTag prepCustomFont(std::string_view name, FT_Face face,
                           FCharFilter filter=AllCharFilter, bool subpixel=false,
                           float points=10.0, uint32_t dpi=72);

    FontTag prepMainFont(FCharFilter filter=AllCharFilter,
                         bool subpixel=false, float points=10.0, uint32_t dpi=72)
    {return prepCustomFont("droidsans-permissive", m_regFace, filter, subpixel, points, dpi);}

    FontTag prepMonoFont(FCharFilter filter=AllCharFilter,
                         bool subpixel=false, float points=10.0, uint32_t dpi=72)
    {return prepCustomFont("bmonofont", m_monoFace, filter, subpixel, points, dpi);}

    FontTag prepCurvesFont(FCharFilter filter=AllCharFilter,
                           bool subpixel=false, float points=10.0, uint32_t dpi=72)
    {return prepCustomFont("spectercurves", m_curvesFace, filter, subpixel, points, dpi);}

    void closeBuiltinFonts() {m_regFace.close(); m_monoFace.close(); m_curvesFace.close();}

    const FontAtlas& lookupAtlas(FontTag tag) const;

    void destroyAtlases() {m_cachedAtlases.clear();}
};

}

