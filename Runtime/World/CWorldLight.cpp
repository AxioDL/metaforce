#include "CWorldLight.hpp"

namespace urde
{
CWorldLight::CWorldLight(CInputStream& in)
: x0_type(EWorldLightType(in.readUint32Big()))
, x4_color(zeus::CVector3f::ReadBig(in))
, x10_position(zeus::CVector3f::ReadBig(in))
, x1c_direction(zeus::CVector3f::ReadBig(in))
, x28_q(in.readFloatBig())
, x2c_cutoffAngle(in.readFloatBig())
, x30_(in.readFloatBig())
, x34_castShadows(in.readBool())
, x38_(in.readFloatBig())
, x3c_falloff(EFalloffType(in.readUint32Big()))
, x40_(in.readFloatBig())
{
}

std::tuple<float, float, float> CalculateLightFalloff(EFalloffType falloff, float q)
{
    float constant = 0.f;
    float linear = 0.f;
    float quadratic = 0.f;

    if (falloff == EFalloffType::Constant)
        constant = 2.f / q;
    else if (falloff == EFalloffType::Linear)
        linear = 250.f / q;
    else if (falloff == EFalloffType::Quadratic)
        quadratic = 25000.f / q;

    return {constant, linear, quadratic};
}

CLight CWorldLight::GetAsCGraphicsLight() const
{
    zeus::CVector3f float_color = x4_color;
    zeus::CColor tmpColor;
    float q = x28_q;
    if (q < FLT_EPSILON)
        q = 0.000001f;

    if (x0_type == EWorldLightType::LocalAmbient)
    {
        float_color *= q;
        if (float_color.x >= 1.f)
            float_color.x = 1.f;

        if (float_color.y >= 1.f)
            float_color.y = 1.f;

        if (float_color.z >= 1.f)
            float_color.z = 1.f;

        return CLight::BuildLocalAmbient(x10_position, zeus::CColor(float_color.x, float_color.y, float_color.z, 1.f));
    }
    else if (x0_type == EWorldLightType::Directional)
    {
        return CLight::BuildDirectional(x1c_direction, tmpColor);
    }
    else if (x0_type == EWorldLightType::Spot)
    {
        CLight light = CLight::BuildSpot(x10_position, x1c_direction.normalized(), tmpColor, x2c_cutoffAngle * .5f);

        float c, l, q;
        std::tie(c, l, q) = CalculateLightFalloff(x3c_falloff, x28_q);

        light.SetAttenuation(c, l, q);
        return light;
    }
    float distC, distL, distQ;
    std::tie(distC, distL, distQ) = CalculateLightFalloff(x3c_falloff, x28_q);
    return CLight::BuildCustom(x10_position, zeus::CVector3f{0.f, 1.f, 0.f},
                               zeus::CColor{x4_color.x, x4_color.y, x4_color.z, 1.f}, distC, distL, distQ, 1.f, 0.f,
                               0.f);
}
}
