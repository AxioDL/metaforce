#include "CFontImageDef.hpp"

namespace urde
{

CFontImageDef::CFontImageDef(std::vector<TToken<CTexture>>&& texs,
                             float interval, const zeus::CVector2f& vec)
: x0_fps(interval), x4_texs(std::move(texs)), x14_pointsPerTexel(vec)
{
    for (TToken<CTexture>& tok : x4_texs)
        tok.Lock();
}

CFontImageDef::CFontImageDef(TToken<CTexture>&& tex, const zeus::CVector2f& vec)
: x0_fps(0.f), x4_texs({std::move(tex)}), x14_pointsPerTexel(vec)
{
    x4_texs[0].Lock();
}

bool CFontImageDef::IsLoaded() const
{
    for (const TToken<CTexture>& tok : x4_texs)
        if (!tok.IsLoaded())
            return false;
    return true;
}

}
