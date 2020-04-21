#pragma once

#include "Runtime/RetroTypes.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {

enum class ELightType {
  Spot = 0,
  Point = 1,
  Directional = 2,
  LocalAmbient = 3,
  Custom = 4,
};
enum class EFalloffType { Constant, Linear, Quadratic };

class CLight {
  friend class CGuiLight;
  friend class CBooModel;
  friend class CBooRenderer;
  friend class CGameLight;

  zeus::CVector3f x0_pos;
  zeus::CVector3f xc_dir = zeus::skDown;
  zeus::CColor x18_color = zeus::skClear;
  ELightType x1c_type = ELightType::Custom;
  float x20_spotCutoff = 0.f;
  float x24_distC = 1.f;
  float x28_distL = 0.f;
  float x2c_distQ = 0.f;
  float x30_angleC = 1.f;
  float x34_angleL = 0.f;
  float x38_angleQ = 0.f;
  u32 x3c_priority = 0;
  u32 x40_lightId = 0; // Serves as unique key
  mutable float x44_cachedRadius = 0.f;
  mutable float x48_cachedIntensity = 0.f;
  mutable bool x4c_24_intensityDirty : 1 = true;
  mutable bool x4c_25_radiusDirty : 1 = true;

  float CalculateLightRadius() const;

public:
  CLight() = default;

  CLight(const zeus::CVector3f& pos, const zeus::CVector3f& dir, const zeus::CColor& color, float distC, float distL,
         float distQ, float angleC, float angleL, float angleQ);

  CLight(ELightType type, const zeus::CVector3f& pos, const zeus::CVector3f& dir, const zeus::CColor& color,
         float cutoff);

  void SetPosition(const zeus::CVector3f& pos) { x0_pos = pos; }

  const zeus::CVector3f& GetPosition() const { return x0_pos; }

  void SetDirection(const zeus::CVector3f& dir) { xc_dir = dir; }

  const zeus::CVector3f& GetDirection() const { return xc_dir; }

  void SetColor(const zeus::CColor& col) {
    x18_color = col;
    x4c_24_intensityDirty = true;
    x4c_25_radiusDirty = true;
  }

  void SetAttenuation(float constant, float linear, float quadratic) {
    x24_distC = constant;
    x28_distL = linear;
    x2c_distQ = quadratic;
    x4c_24_intensityDirty = true;
    x4c_25_radiusDirty = true;
  }
  float GetAttenuationConstant() const { return x24_distC; }
  float GetAttenuationLinear() const { return x28_distL; }
  float GetAttenuationQuadratic() const { return x2c_distQ; }

  void SetAngleAttenuation(float constant, float linear, float quadratic) {
    x30_angleC = constant;
    x34_angleL = linear;
    x38_angleQ = quadratic;
    x4c_24_intensityDirty = true;
    x4c_25_radiusDirty = true;
  }
  float GetAngleAttenuationConstant() const { return x30_angleC; }
  float GetAngleAttenuationLinear() const { return x34_angleL; }
  float GetAngleAttenuationQuadratic() const { return x38_angleQ; }

  ELightType GetType() const { return x1c_type; }

  float GetIntensity() const;
  float GetRadius() const;
  const zeus::CColor& GetColor() const { return x18_color; }
  zeus::CColor GetNormalIndependentLightingAtPoint(const zeus::CVector3f& point) const;

  static CLight BuildDirectional(const zeus::CVector3f& dir, const zeus::CColor& color);
  static CLight BuildSpot(const zeus::CVector3f& pos, const zeus::CVector3f& dir, const zeus::CColor& color,
                          float angle);
  static CLight BuildPoint(const zeus::CVector3f& pos, const zeus::CColor& color);
  static CLight BuildCustom(const zeus::CVector3f& pos, const zeus::CVector3f& dir, const zeus::CColor& color,
                            float distC, float distL, float distQ, float angleC, float angleL, float angleQ);
  static CLight BuildLocalAmbient(const zeus::CVector3f& pos, const zeus::CColor& color);
};

} // namespace urde
