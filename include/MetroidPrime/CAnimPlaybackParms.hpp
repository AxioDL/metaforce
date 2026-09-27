#ifndef _CANIMPLAYBACKPARMS
#define _CANIMPLAYBACKPARMS

#include "types.h"

class CQuaternion;
class CTransform4f;
class CVector3f;

class CAnimPlaybackParms {
private:
  int mAnimA;
  int mAnimB;
  float mBlendWeight;
  bool mAnimating;
  int x10_;
  const CVector3f* mTargetPos;
  bool mUseLocator;
  const CQuaternion* mDeltaOrient;
  const CTransform4f* mObjectXf;
  const CVector3f* mObjectScale;

public:
  CAnimPlaybackParms(const int animA, const int animB, const float blendWeight,
                     const bool animating)
  : mAnimA(animA)
  , mAnimB(animB)
  , mBlendWeight(blendWeight)
  , mAnimating(animating)
  , x10_(0)
  , mTargetPos(nullptr)
  , mUseLocator(false)
  , mDeltaOrient(nullptr)
  , mObjectXf(nullptr)
  , mObjectScale(nullptr) {}

  CAnimPlaybackParms(int anim, const CQuaternion* deltaOrient, const CVector3f* targetPos,
                     const CTransform4f* xf, const CVector3f* scale, bool useLocator)
  : mAnimA(anim)
  , mAnimB(-1)
  , mBlendWeight(1.f)
  , mAnimating(true)
  , x10_(0)
  , mTargetPos(targetPos)
  , mUseLocator(useLocator)
  , mDeltaOrient(deltaOrient)
  , mObjectXf(xf)
  , mObjectScale(scale) {}

  int GetAnimationId() const { return mAnimA; }
  int GetSecondAnimationId() const { return mAnimB; }
  float GetBlendFactor() const { return mBlendWeight; }
  bool GetIsPlayAnimation() const { return mAnimating; }
  const CVector3f* GetTargetPos() const { return mTargetPos; }
  bool GetIsUseLocator() const { return mUseLocator; }
  const CQuaternion* GetDeltaOrient() const { return mDeltaOrient; }
  const CTransform4f* GetObjectXform() const { return mObjectXf; }
  const CVector3f* GetObjectScale() const { return mObjectScale; }

  void SetAnimationId(int id) { mAnimA = id; }
  void SetSecondAnimationId(int id) { mAnimB = id; }
  void SetBlendFactor(float f) { mBlendWeight = f; }
};
CHECK_SIZEOF(CAnimPlaybackParms, 0x28)

#endif // _CANIMPLAYBACKPARMS
