#pragma once

#include "Runtime/RetroTypes.hpp"

#include <zeus/CQuaternion.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CAnimPlaybackParms {
  s32 x0_animA = -1;
  s32 x4_animB = -1;
  float x8_blendWeight = 1.f;
  bool xc_animating = true;
  // s32 x10_ = 0;
  const zeus::CVector3f* x14_targetPos = nullptr;
  bool x18_useLocator = false;
  const zeus::CQuaternion* x1c_deltaOrient = nullptr;
  const zeus::CTransform* x20_objectXf = nullptr;
  const zeus::CVector3f* x24_objectScale = nullptr;

public:
  constexpr CAnimPlaybackParms() = default;
  constexpr CAnimPlaybackParms(s32 animA, s32 animB, float blendWeight, bool animating)
  : x0_animA(animA), x4_animB(animB), x8_blendWeight(blendWeight), xc_animating(animating) {}
  constexpr CAnimPlaybackParms(s32 anim, const zeus::CQuaternion* deltaOrient, const zeus::CVector3f* targetPos,
                               const zeus::CTransform* xf, const zeus::CVector3f* scale, bool useLocator)
  : x0_animA(anim)
  , x14_targetPos(targetPos)
  , x18_useLocator(useLocator)
  , x1c_deltaOrient(deltaOrient)
  , x20_objectXf(xf)
  , x24_objectScale(scale) {}

  constexpr const zeus::CTransform* GetObjectXform() const { return x20_objectXf; }
  constexpr const zeus::CQuaternion* GetDeltaOrient() const { return x1c_deltaOrient; }
  constexpr const zeus::CVector3f* GetTargetPos() const { return x14_targetPos; }
  constexpr bool GetIsUseLocator() const { return x18_useLocator; }
  constexpr const zeus::CVector3f* GetObjectScale() const { return x24_objectScale; }
  constexpr s32 GetAnimationId() const { return x0_animA; }
  constexpr s32 GetSecondAnimationId() const { return x4_animB; }
  constexpr float GetBlendFactor() const { return x8_blendWeight; }
  constexpr void SetAnimationId(s32 id) { x0_animA = id; }
  constexpr void SetSecondAnimationId(s32 id) { x4_animB = id; }
  constexpr void SetBlendFactor(float f) { x8_blendWeight = f; }
  constexpr bool GetIsPlayAnimation() const { return xc_animating; }
};
} // namespace metaforce
