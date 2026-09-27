#include "Kyoto/Animation/CAdditiveAnimPlayback.hpp"

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/CSegStatementSet.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "rstl/math.hpp"

CAdditiveAnimPlayback::CAdditiveAnimPlayback(const rstl::ncrc_ptr< CAnimTreeNode >& anim,
                                             float weight, bool loop,
                                             const CAdditiveAnimationInfo& info, bool fadeOut)
: mInfo(info)
, mAnim(anim)
, mTargetWeight(rstl::max_val(0.f, rstl::min_val(weight, 1.f)))
, mCurWeight(0.f)
, mActive(loop)
, mWeightTimer(0.f)
, mPhase(kPP_FadingIn)
, mNeedsFadeOut(!loop && fadeOut) {}

void CAdditiveAnimPlayback::SetWeight(float weight) {
  mTargetWeight = rstl::max_val(0.f, rstl::min_val(weight, 1.f));

  if (mPhase == kPP_FadingIn) {
    mCurWeight = mInfo.GetFadeInTime() > 0.f
                        ? mTargetWeight * (mWeightTimer / mInfo.GetFadeInTime())
                        : mTargetWeight;
    return;
  }
  if (mPhase == kPP_FadingOut) {
    mCurWeight = mInfo.GetFadeOutTime() > 0.f
                        ? mTargetWeight * (mWeightTimer / mInfo.GetFadeOutTime())
                        : mTargetWeight;
    return;
  }
  mCurWeight = mTargetWeight;
}

void CAdditiveAnimPlayback::FadeOut() {
  if (mPhase == kPP_FadedOut || mPhase == kPP_FadedIn) {
    mWeightTimer = mInfo.GetFadeOutTime();
  } else if (mPhase == kPP_FadingIn) {
    mWeightTimer = (mWeightTimer / mInfo.GetFadeInTime()) * mInfo.GetFadeOutTime();
  }

  if (mInfo.GetFadeOutTime() > 0.f) {
    mPhase = kPP_FadingOut;
    return;
  }

  mPhase = kPP_FadedOut;
  mCurWeight = 0.f;
}

void CAdditiveAnimPlayback::Update(float dt) {
  if (mPhase == kPP_FadingIn) {
    const float time = mInfo.GetFadeInTime();
    mWeightTimer = rstl::min_val(time, mWeightTimer + dt);
    mCurWeight = time > 0.f ? mTargetWeight * (mWeightTimer / time) : mTargetWeight;

    if (close_enough(mCurWeight, mTargetWeight)) {
      mPhase = kPP_FadedIn;
    }
  } else if (mPhase == kPP_FadingOut) {
    const float time = mInfo.GetFadeOutTime();
    mWeightTimer = rstl::max_val(0.f, mWeightTimer - dt);
    mCurWeight = time > 0.f ? mTargetWeight * (mWeightTimer / time) : 0.f;
    if (close_enough(mCurWeight, 0.f)) {
      mPhase = kPP_FadedOut;
    }
  }
}

void CAdditiveAnimPlayback::AddToSegStatementSet(const CSegIdList& list,
                                                 const CCharLayoutInfo& charInfo,
                                                 CSegStatementSet& set) const {
  CStackSegStatementSet stackSegSet;

  mAnim->VGetSegStatementSet(list, stackSegSet);

  const size_t count = list.Size();
  for (size_t i = 0; i < count; ++i) {
    const CSegId seg = list[i];
    stackSegSet.Set(list[i], charInfo.GetFromParentUnrotated(seg));
  }

  set.Add(list, charInfo, stackSegSet, mCurWeight);
}