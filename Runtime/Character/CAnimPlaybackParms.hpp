#ifndef __CANIMPLAYBACKPARMS_HPP__
#define __CANIMPLAYBACKPARMS_HPP__

#include "RetroTypes.hpp"
#include "zeus/CQuaternion.hpp"

namespace urde
{
class CAnimPlaybackParms
{
    s32   x0_animA = -1;
    s32   x4_animB = -1;
    float x8_blendWeight = 1.f;
    bool  xc_animating = true;
    s32 x10_ = 0;
    const zeus::CVector3f* x14_targetPos = nullptr;
    bool  x18_useLocator = false;
    const zeus::CQuaternion* x1c_deltaOrient = nullptr;
    const zeus::CTransform* x20_objectXf = nullptr;
    const zeus::CVector3f* x24_objectScale = nullptr;
public:
    CAnimPlaybackParms() = default;
    CAnimPlaybackParms(s32 animA, s32 animB, float blendWeight, bool animating)
    : x0_animA(animA), x4_animB(animB), x8_blendWeight(blendWeight), xc_animating(animating) {}
    CAnimPlaybackParms(s32 anim, const zeus::CQuaternion* deltaOrient, const zeus::CVector3f* targetPos,
                       const zeus::CTransform* xf, const zeus::CVector3f* scale, bool useLocator)
    : x0_animA(anim), x14_targetPos(targetPos), x18_useLocator(useLocator), x1c_deltaOrient(deltaOrient),
      x20_objectXf(xf), x24_objectScale(scale) {}
    const zeus::CTransform* GetObjectXform() const { return x20_objectXf; }
    const zeus::CQuaternion* GetDeltaOrient() const { return x1c_deltaOrient; }
    const zeus::CVector3f* GetTargetPos() const { return x14_targetPos; }
    bool GetIsUseLocator() const { return x18_useLocator; }
    const zeus::CVector3f* GetObjectScale() const { return x24_objectScale; }
    s32 GetAnimationId() const { return x0_animA; }
    s32 GetSecondAnimationId() const { return x4_animB; }
    float GetBlendFactor() const { return x8_blendWeight; }
    void SetAnimationId(s32 id) { x0_animA = id; }
    void SetSecondAnimationId(s32 id) { x4_animB = id; }
    void SetBlendFactor(float f) { x8_blendWeight = f; }
    bool GetIsPlayAnimation() const { return xc_animating; }
};
}

#endif // CANIMPLAYBACKPARMS_HPP
