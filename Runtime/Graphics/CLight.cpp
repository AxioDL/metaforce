#include "CLight.hpp"

namespace urde
{

CLight CLight::BuildDirectional(const zeus::CVector3f& dir, const zeus::CColor& color)
{
    return {};
}

CLight CLight::BuildSpot(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                         const zeus::CColor& color, float angle)
{
    return {};
}

CLight CLight::BuildCustom(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                           const zeus::CColor& color, float constAtt, float linearAtt, float quadAtt,
                           float intensity, float, float)
{
    return {};
}

}
