#ifndef __PSHAG_CLIGHT_HPP__
#define __PSHAG_CLIGHT_HPP__

#include "CVector3f.hpp"
#include "CColor.hpp"

namespace pshag
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
    static CLight BuildDirectional(const Zeus::CVector3f& dir, const Zeus::CColor& color);
    static CLight BuildSpot(const Zeus::CVector3f& pos, const Zeus::CVector3f& dir,
                            const Zeus::CColor& color, float angle);
    static CLight BuildCustom(const Zeus::CVector3f& pos, const Zeus::CVector3f& dir,
                              const Zeus::CColor& color, float constAtt, float linearAtt, float quadAtt,
                              float intensity, float, float);
};

}

#endif // __PSHAG_CLIGHT_HPP__
