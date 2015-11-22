#ifndef SPECTER_FONTCACHE_HPP
#define SPECTER_FONTCACHE_HPP

#include <ft2build.h>
#include FT_FREETYPE_H

#include <boo/boo.hpp>
#include <HECL/Runtime.hpp>

namespace Specter
{

class FreeTypeGZipMemFace
{
    FT_StreamRec m_comp = {};
    FT_StreamRec m_decomp = {};
    FT_Face m_face;
public:
    FreeTypeGZipMemFace(FT_Library lib, const uint8_t* data, size_t sz);
    ~FreeTypeGZipMemFace();
    operator FT_Face() {return m_face;}
};

class FontHandle
{
};

class FontAtlas
{
    FT_Face m_face;
    std::vector<boo::ITextureS*> m_texs;
};

class FontCache
{
    const HECL::Runtime::FileStoreManager& m_fileMgr;
    struct Library
    {
        FT_Library m_lib;
        Library();
        ~Library();
        operator FT_Library() {return m_lib;}
    } m_fontLib;
    FreeTypeGZipMemFace m_regFace;
    FreeTypeGZipMemFace m_monoFace;
public:
    FontCache(const HECL::Runtime::FileStoreManager& fileMgr);

    FontHandle prepMainFont(float points=10.0);
    FontHandle prepMonoFont(float points=10.0);
    FontHandle prepCustomFont(FT_Face face, float points=10.0);
};

}

#endif // SPECTER_FONTCACHE_HPP
