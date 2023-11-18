#pragma once

#include <memory>
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Streams/CInputStream.hpp"

namespace metaforce {
class CAdditiveAnimationInfo;
class CAnimTreeNode;
class CCharLayoutInfo;
class CSegIdList;
class CSegStatementSet;

class CAdditiveAnimationInfo {
  float x0_fadeInDur = 0.f;
  float x4_fadeOutDur = 0.f;

public:
  void read(CInputStream& in) {
    x0_fadeInDur = in.ReadFloat();
    x4_fadeOutDur = in.ReadFloat();
  }
  CAdditiveAnimationInfo() = default;
  explicit CAdditiveAnimationInfo(CInputStream& in) { read(in); }
  float GetFadeInDuration() const { return x0_fadeInDur; }
  float GetFadeOutDuration() const { return x4_fadeOutDur; }
};

enum class EAdditivePlaybackPhase { None, FadingIn, FadingOut, FadedIn, FadedOut };

class CAdditiveAnimPlayback {
  CAdditiveAnimationInfo x0_info;
  std::shared_ptr<CAnimTreeNode> x8_anim;
  float xc_targetWeight;
  float x10_curWeight = 0.f;
  bool x14_active;
  float x18_weightTimer = 0.f;
  EAdditivePlaybackPhase x1c_phase = EAdditivePlaybackPhase::FadingIn;
  bool x20_needsFadeOut = false;

public:
  CAdditiveAnimPlayback(const std::weak_ptr<CAnimTreeNode>& anim, float weight, bool active,
                        const CAdditiveAnimationInfo& info, bool fadeOut);

  void AddToSegStatementSet(const CSegIdList& list, const CCharLayoutInfo&, CSegStatementSet&) const;
  void Update(float dt);
  void FadeOut();
  void SetWeight(float w);
  float GetTargetWeight() const { return xc_targetWeight; }
  bool IsActive() const { return x14_active; }
  void SetActive(bool active) { x14_active = active; }
  const std::shared_ptr<CAnimTreeNode>& GetAnim() const { return x8_anim; }
  std::shared_ptr<CAnimTreeNode>& GetAnim() { return x8_anim; }
  EAdditivePlaybackPhase GetPhase() const { return x1c_phase; }
  void SetNeedsFadeOut(bool b) { x20_needsFadeOut = b; }
  bool NeedsFadeOut() const { return x20_needsFadeOut; }
};

} // namespace metaforce
