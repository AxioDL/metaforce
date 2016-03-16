#include "CLight.hpp"
#include <cfloat>

namespace urde
{

static const zeus::CVector3f kDefaultPosition(0.f, 0.f, 0.f);
static const zeus::CVector3f kDefaultDirection(0.f, -1.f, 0.f);

float CLight::CalculateLightRadius() const
{
    if (x28_distL < FLT_EPSILON && x2c_distQ < FLT_EPSILON)
        return FLT_MAX;

    float intens = GetIntensity();

    if (x2c_distQ > FLT_EPSILON)
    {
        if (intens <= FLT_EPSILON)
            return 0.f;
        return std::sqrt(intens / 5.f * intens / 255.f * x2c_distQ);
    }

    float nextIntens = 5.f * intens /  255.f;
    return intens / std::min(0.2f, nextIntens) * x28_distL;
}

float CLight::GetIntensity() const
{
    if (x4c_24_intensityDirty)
    {
        ((CLight*)this)->x4c_24_intensityDirty = false;
        float coef = 1.f;
        if (x1c_type == ELightType::Custom)
            coef = x30_angleC;
        ((CLight*)this)->x48_cachedIntensity =
            coef * std::max(x18_color.r, std::max(x18_color.g, x18_color.b));
    }
    return x48_cachedIntensity;
}

CLight CLight::BuildDirectional(const zeus::CVector3f& dir, const zeus::CColor& color)
{
    return CLight(ELightType::Directional, kDefaultPosition, dir, color, 180.f);
}

CLight CLight::BuildSpot(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                         const zeus::CColor& color, float angle)
{
    return CLight(ELightType::Spot, pos, dir, color, angle);
}

CLight CLight::BuildPoint(const zeus::CVector3f& pos, const zeus::CColor& color)
{
    return CLight(ELightType::Point, pos, kDefaultDirection, color, 180.f);
}

CLight CLight::BuildCustom(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                           const zeus::CColor& color,
                           float distC, float distL, float distQ,
                           float angleC, float angleL, float angleQ)
{
    return CLight(pos, dir, color, distC, distL, distQ, angleC, angleL, angleQ);
}

CLight CLight::BuildLocalAmbient(const zeus::CVector3f& pos, const zeus::CColor& color)
{
    return CLight(ELightType::LocalAmbient, pos, kDefaultDirection, color, 180.f);
}

}
