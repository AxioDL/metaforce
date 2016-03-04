#ifndef __PSHAG_CLIGHT_HPP__
#define __PSHAG_CLIGHT_HPP__

#include "zeus/CVector3f.hpp"
#include "zeus/CColor.hpp"

namespace urde
{

enum class ELightType
{
    LocalAmbient,
    Directional,
    Custom,
    Spot,
    Spot2,
    LocalAmbient2
};
enum class EFalloffType
{
    Constant,
    Linear,
    Quadratic
};

class CLight
{
public:
    static CLight BuildDirectional(const zeus::CVector3f& dir, const zeus::CColor& color);
    static CLight BuildSpot(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                            const zeus::CColor& color, float angle);
    static CLight BuildCustom(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                              const zeus::CColor& color, float constAtt, float linearAtt, float quadAtt,
                              float intensity, float, float);
};

}

#endif // __PSHAG_CLIGHT_HPP__
