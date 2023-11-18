#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CAdditiveAnimPlayback.hpp"
#include "Runtime/Character/CAnimation.hpp"
#include "Runtime/Character/CHalfTransition.hpp"
#include "Runtime/Character/CTransition.hpp"

namespace metaforce {

class CAnimationSet {
  u16 x0_tableCount;
  std::vector<CAnimation> x4_animations;
  std::vector<CTransition> x14_transitions;
  std::shared_ptr<IMetaTrans> x24_defaultTransition;
  std::vector<std::pair<u32, CAdditiveAnimationInfo>> x28_additiveInfo;
  CAdditiveAnimationInfo x38_defaultAdditiveInfo;
  std::vector<CHalfTransition> x40_halfTransitions;
  std::vector<std::pair<CAssetId, CAssetId>> x50_animRes;

public:
  explicit CAnimationSet(CInputStream& in);

  const std::vector<CAnimation>& GetAnimations() const { return x4_animations; }
  const std::vector<CTransition>& GetTransitions() const { return x14_transitions; }
  const std::shared_ptr<IMetaTrans>& GetDefaultTransition() const { return x24_defaultTransition; }
  const std::vector<CHalfTransition>& GetHalfTransitions() const { return x40_halfTransitions; }
  const std::vector<std::pair<u32, CAdditiveAnimationInfo>>& GetAdditiveInfo() const { return x28_additiveInfo; }
  const CAdditiveAnimationInfo& GetDefaultAdditiveInfo() const { return x38_defaultAdditiveInfo; }
  const std::vector<std::pair<CAssetId, CAssetId>>& GetAnimResIds() const { return x50_animRes; }
};
} // namespace metaforce
