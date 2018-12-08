#pragma once

#include "Graphics/CLight.hpp"

namespace urde {
class CWorldLight {
public:
  enum class EWorldLightType {
    LocalAmbient,
    Directional,
    Custom,
    Spot,
    Spot2,
    LocalAmbient2,
  };

private:
  EWorldLightType x0_type = EWorldLightType::Spot2;
  zeus::CVector3f x4_color;
  zeus::CVector3f x10_position;
  zeus::CVector3f x1c_direction;
  float x28_q = 0.f;
  float x2c_cutoffAngle = 0.f;
  float x30_ = 0.f;
  bool x34_castShadows = false;
  float x38_ = 0.f;
  EFalloffType x3c_falloff = EFalloffType::Linear;
  float x40_ = 0.f;

public:
  CWorldLight(const CWorldLight&) = default;
  CWorldLight(CInputStream& in);
  EWorldLightType GetLightType() const { return x0_type; }
  const zeus::CVector3f& GetDirection() const { return x1c_direction; }
  const zeus::CVector3f& GetPosition() const { return x10_position; }
  bool DoesCastShadows() const { return x34_castShadows; }

  CLight GetAsCGraphicsLight() const;
};

} // namespace urde
