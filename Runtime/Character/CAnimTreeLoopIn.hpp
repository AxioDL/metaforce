#pragma once

#include "CAnimTreeSingleChild.hpp"
#include "CAnimSysContext.hpp"
#include "CSequenceHelper.hpp"

namespace urde {

class CAnimTreeLoopIn : public CAnimTreeSingleChild {
  std::shared_ptr<CAnimTreeNode> x18_nextAnim;
  bool x1c_didLoopIn = false;
  CAnimSysContext x20_animCtx;
  CSequenceFundamentals x30_fundamentals;
  CCharAnimTime x88_curTime;

public:
  static std::string CreatePrimitiveName(const std::weak_ptr<CAnimTreeNode>& a, const std::weak_ptr<CAnimTreeNode>& b,
                                         const std::weak_ptr<CAnimTreeNode>& c);
  CAnimTreeLoopIn(const std::weak_ptr<CAnimTreeNode>& a, const std::weak_ptr<CAnimTreeNode>& b,
                  const std::weak_ptr<CAnimTreeNode>& c, const CAnimSysContext& animCtx, std::string_view name);
  CAnimTreeLoopIn(const std::weak_ptr<CAnimTreeNode>& a, const std::weak_ptr<CAnimTreeNode>& b, bool didLoopIn,
                  const CAnimSysContext& animCtx, std::string_view name, const CSequenceFundamentals& fundamentals,
                  const CCharAnimTime& time);
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const;
  bool VSupportsReverseView() const { return false; }
  rstl::optional<std::unique_ptr<IAnimReader>> VSimplified();
  std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const;
  std::unique_ptr<IAnimReader> VClone() const;
  u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const;
  u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator, u32) const;
  u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator, u32) const;
  u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator, u32) const;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const;
  CCharAnimTime VGetTimeRemaining() const;
  SAdvancementResults VAdvanceView(const CCharAnimTime& dt);
};

} // namespace urde
