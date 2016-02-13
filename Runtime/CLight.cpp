#include "CLight.hpp"

namespace pshag
{

CLight CLight::BuildDirectional(const Zeus::CVector3f& dir, const Zeus::CColor& color)
{
    return {};
}

CLight CLight::BuildSpot(const Zeus::CVector3f& pos, const Zeus::CVector3f& dir,
                         const Zeus::CColor& color, float angle)
{
    return {};
}

CLight CLight::BuildCustom(const Zeus::CVector3f& pos, const Zeus::CVector3f& dir,
                           const Zeus::CColor& color, float constAtt, float linearAtt, float quadAtt,
                           float intensity, float, float)
{
    return {};
}

}
