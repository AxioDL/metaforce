#pragma once

#include "Runtime/CToken.hpp"
#include "Runtime/Graphics/CTexture.hpp"

#include <optional>

#include <zeus/CAABox.hpp>
#include <zeus/CTransform.hpp>

namespace metaforce {
class CTexture;
class CStateManager;

class CSimpleShadow {
  zeus::CTransform x0_xf;
  float x30_scale;
  float x34_radius = 1.f;
  float x38_userAlpha;
  float x3c_heightAlpha = 1.f;
  float x40_maxObjHeight;
  float x44_displacement;
  bool x48_24_collision : 1 = false;
  bool x48_25_alwaysCalculateRadius : 1 = true;
  bool x48_26_radiusCalculated : 1 = false;

public:
  CSimpleShadow(float scale, float userAlpha, float maxObjHeight, float displacement);
  bool Valid() const { return x48_24_collision; }
  zeus::CAABox GetMaxShadowBox(const zeus::CAABox& aabb) const;
  zeus::CAABox GetBounds() const;
  void SetAlwaysCalculateRadius(bool) { x48_25_alwaysCalculateRadius = true; }
  float GetMaxObjectHeight() const { return x40_maxObjHeight; }
  void SetUserAlpha(float a) { x38_userAlpha = a; }
  const zeus::CTransform& GetTransform() const { return x0_xf; }
  void Render(TLockedToken<CTexture>& tex);
  void Calculate(const zeus::CAABox& aabb, const zeus::CTransform& xf, const CStateManager& mgr);
};
} // namespace metaforce
