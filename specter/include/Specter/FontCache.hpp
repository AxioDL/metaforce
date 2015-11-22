#ifndef SPECTER_FONTCACHE_HPP
#define SPECTER_FONTCACHE_HPP

#include <ft2build.h>
#include FT_FREETYPE_H

#include <boo/boo.hpp>
#include <HECL/Runtime.hpp>

namespace Specter
{

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
    FT_Library m_fontLib;
    FT_Face m_regFace;
    FT_Face m_monoFace;
public:
    FontCache(const HECL::Runtime::FileStoreManager& fileMgr);
    ~FontCache();

    FontHandle prepMainFont(float points=10.0);
    FontHandle prepMonoFont(float points=10.0);
    FontHandle prepCustomFont(FT_Face face, float points=10.0);
};

}

#endif // SPECTER_FONTCACHE_HPP
