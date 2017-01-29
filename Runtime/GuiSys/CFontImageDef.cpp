#include "CFontImageDef.hpp"

namespace urde
{

CFontImageDef::CFontImageDef(const std::vector<TToken<CTexture>>& texs,
                             float interval, const zeus::CVector2f& vec)
: x0_fps(interval), x14_pointsPerTexel(vec)
{
    x4_texs.reserve(texs.size());
    for (const TToken<CTexture>& tok : texs)
        x4_texs.push_back(tok);
}

CFontImageDef::CFontImageDef(const TToken<CTexture>& tex, const zeus::CVector2f& vec)
: x0_fps(0.f), x14_pointsPerTexel(vec)
{
    x4_texs.push_back(tex);
}

bool CFontImageDef::IsLoaded() const
{
    for (const TToken<CTexture>& tok : x4_texs)
        if (!tok.IsLoaded())
            return false;
    return true;
}

}
