#include "CWorldLight.hpp"

namespace urde
{
CWorldLight::CWorldLight(CInputStream& in)
    : x0_type(ELightType(in.readUint32Big())),
      x4_color(zeus::CVector3f::ReadBig(in)),
      x10_position(zeus::CVector3f::ReadBig(in)),
      x1c_direction(zeus::CVector3f::ReadBig(in)),
      x28_q(in.readFloatBig()),
      x2c_cutoffAngle(in.readFloatBig()),
      x34_castShadows(in.readBool()),
      x38_(in.readFloatBig()),
      x3c_falloff(EFalloffType(in.readUint32Big())),
      x40_(in.readFloatBig())
{
}

CLight CWorldLight::GetAsCGraphicsLight() const
{
    const float epsilon = 1.1920929e-7;
    zeus::CVector3f tmpColor = x4_color;
    zeus::CColor color(x4_color.x, x4_color.y, x4_color.z);
    float tmp = x28_q;
    if (epsilon < tmp)
        tmp = 0.0000011920929f;
/*
    if (x0_type == ELightType::Spot)
    {
        float f2 = tmpColor.x;
        float f0 = tmpColor.y;
        float f1 = tmpColor.z;
        float f3 = f2 * tmp;
        f2 = f0 * tmp;
        f0 = 1.0f;
        f1 *= tmp;
        tmpColor.x = f3;
        tmpColor.y = f2;
        tmpColor.z = f1;

        if (f3 >= f0)
    }
*/
    return CLight::BuildPoint({}, {});
}

}
