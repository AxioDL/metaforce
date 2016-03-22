#ifndef __URDE_CFONTIMAGEDEF_HPP__
#define __URDE_CFONTIMAGEDEF_HPP__

#include <vector>
#include "CToken.hpp"
#include "zeus/CVector2f.hpp"

namespace urde
{
class CTexture;

class CFontImageDef
{
public:
    float x0_fps;
    std::vector<TToken<CTexture>> x4_texs;
    zeus::CVector2f x14_pointsPerTexel;

    CFontImageDef(std::vector<TToken<CTexture>>&& texs, float fps,
                  const zeus::CVector2f& vec);
    CFontImageDef(TToken<CTexture>&& tex, const zeus::CVector2f& vec);
    bool IsLoaded() const;
};

}

#endif // __URDE_CFONTIMAGEDEF_HPP__
