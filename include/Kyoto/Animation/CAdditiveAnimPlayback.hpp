#ifndef _CADDITIVEANIMPLAYBACK
#define _CADDITIVEANIMPLAYBACK

#include "types.h"

#include "Kyoto/Animation/CAdditiveAnimationInfo.hpp"

#include "rstl/rc_ptr.hpp"

class CAnimTreeNode;
class CCharLayoutInfo;
class CSegIdList;
class CSegStatementSet;

class CAdditiveAnimPlayback {
public:
  enum EPlaybackPhase {
    kPP_None,
    kPP_FadingIn,
    kPP_FadingOut,
    kPP_FadedIn,
    kPP_FadedOut,
  };

  CAdditiveAnimPlayback(const rstl::ncrc_ptr< CAnimTreeNode >& anim, float weight, bool loop,
                        const CAdditiveAnimationInfo& info, bool fadeOut);

  const rstl::rc_ptr< CAnimTreeNode >& GetAnimationTree() const { return mAnim; }
  rstl::ncrc_ptr< CAnimTreeNode >& AnimationTree() { return mAnim; }

  float GetWeight() const { return mTargetWeight; }
  void SetWeight(float weight);

  EPlaybackPhase GetFadingMode() const { return mPhase; }
  bool IsLoop() const { return mActive; }
  void SetLoop(bool loop) { mActive = loop; }

  bool IsFadeOutWhenAnimOver() const { return mNeedsFadeOut; }
  void SetFadeOutWhenAnimOver(bool fadeOut) { mNeedsFadeOut = fadeOut; }

  void FadeOut();
  void Update(float dt);
  void AddToSegStatementSet(const CSegIdList&, const CCharLayoutInfo&, CSegStatementSet&) const;

private:
  CAdditiveAnimationInfo mInfo;
  rstl::ncrc_ptr< CAnimTreeNode > mAnim;
  float mTargetWeight;
  float mCurWeight;
  bool mActive;
  float mWeightTimer;
  EPlaybackPhase mPhase;
  bool mNeedsFadeOut;
};
CHECK_SIZEOF(CAdditiveAnimPlayback, 0x24)

#endif // _CADDITIVEANIMPLAYBACK
