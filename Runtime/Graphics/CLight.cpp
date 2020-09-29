#include "Runtime/Graphics/CLight.hpp"

#include <cfloat>

namespace urde {

constexpr zeus::CVector3f kDefaultPosition(0.f, 0.f, 0.f);
constexpr zeus::CVector3f kDefaultDirection(0.f, -1.f, 0.f);

float CLight::CalculateLightRadius() const {
  if (x28_distL < FLT_EPSILON && x2c_distQ < FLT_EPSILON)
    return FLT_MAX;

  float intens = GetIntensity();

  if (x2c_distQ > FLT_EPSILON) {
    if (intens <= FLT_EPSILON)
      return 0.f;
    return std::sqrt(intens / (0.0588235f * x2c_distQ));
  }

  if (x28_distL > FLT_EPSILON)
    return intens / (0.0588235f * x28_distL);

  return 0.f;
}

float CLight::GetIntensity() const {
  if (x4c_24_intensityDirty) {
    x4c_24_intensityDirty = false;
    float coef = 1.f;
    if (x1c_type == ELightType::Custom) {
      coef = x30_angleC;
    }
    x48_cachedIntensity = coef * std::max({x18_color.r(), x18_color.g(), x18_color.b()});
  }
  return x48_cachedIntensity;
}

float CLight::GetRadius() const {
  if (x4c_25_radiusDirty) {
    x44_cachedRadius = CalculateLightRadius();
    x4c_25_radiusDirty = false;
  }
  return x44_cachedRadius;
}

CLight::CLight(const zeus::CVector3f& pos, const zeus::CVector3f& dir, const zeus::CColor& color, float distC,
               float distL, float distQ, float angleC, float angleL, float angleQ)
: x0_pos(pos)
, xc_dir(dir)
, x18_color(color)
, x24_distC(distC)
, x28_distL(distL)
, x2c_distQ(distQ)
, x30_angleC(angleC)
, x34_angleL(angleL)
, x38_angleQ(angleQ) {}

CLight::CLight(ELightType type, const zeus::CVector3f& pos, const zeus::CVector3f& dir, const zeus::CColor& color,
               float cutoff)
: x0_pos(pos)
, xc_dir(dir)
, x18_color(color)
, x1c_type(type)
, x20_spotCutoff(cutoff) {
  switch (type) {
  case ELightType::Spot: {
    const float cosCutoff = std::cos(zeus::degToRad(cutoff));
    x30_angleC = 0.f;
    x34_angleL = -cosCutoff / (1.0f - cosCutoff);
    x38_angleQ = 1.f / (1.0f - cosCutoff);
    break;
  }
  case ELightType::Directional: {
    x24_distC = 1.f;
    x28_distL = 0.f;
    x2c_distQ = 0.f;
    break;
  }
  default:
    break;
  }
}

zeus::CColor CLight::GetNormalIndependentLightingAtPoint(const zeus::CVector3f& point) const {
  if (x1c_type == ELightType::LocalAmbient)
    return x18_color;

  float dist = std::max((x0_pos - point).magnitude(), FLT_EPSILON);
  return x18_color * (1.f / (x2c_distQ * dist * dist + x28_distL * dist + x24_distC));
}

CLight CLight::BuildDirectional(const zeus::CVector3f& dir, const zeus::CColor& color) {
  return CLight(ELightType::Directional, kDefaultPosition, dir, color, 180.f);
}

CLight CLight::BuildSpot(const zeus::CVector3f& pos, const zeus::CVector3f& dir, const zeus::CColor& color,
                         float angle) {
  return CLight(ELightType::Spot, pos, dir, color, angle);
}

CLight CLight::BuildPoint(const zeus::CVector3f& pos, const zeus::CColor& color) {
  return CLight(ELightType::Point, pos, kDefaultDirection, color, 180.f);
}

CLight CLight::BuildCustom(const zeus::CVector3f& pos, const zeus::CVector3f& dir, const zeus::CColor& color,
                           float distC, float distL, float distQ, float angleC, float angleL, float angleQ) {
  return CLight(pos, dir, color, distC, distL, distQ, angleC, angleL, angleQ);
}

CLight CLight::BuildLocalAmbient(const zeus::CVector3f& pos, const zeus::CColor& color) {
  return CLight(ELightType::LocalAmbient, pos, kDefaultDirection, color, 180.f);
}

} // namespace urde
