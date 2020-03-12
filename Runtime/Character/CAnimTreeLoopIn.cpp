#include "Runtime/Character/CAnimTreeLoopIn.hpp"

#include "Runtime/Character/CTreeUtils.hpp"

namespace urde {

std::string CAnimTreeLoopIn::CreatePrimitiveName(const std::weak_ptr<CAnimTreeNode>& a,
                                                 const std::weak_ptr<CAnimTreeNode>& b,
                                                 const std::weak_ptr<CAnimTreeNode>& c) {
  return {};
}

CAnimTreeLoopIn::CAnimTreeLoopIn(const std::weak_ptr<CAnimTreeNode>& a, const std::weak_ptr<CAnimTreeNode>& b,
                                 const std::weak_ptr<CAnimTreeNode>& c, const CAnimSysContext& animCtx,
                                 std::string_view name)
: CAnimTreeSingleChild(CTreeUtils::GetTransitionTree(a, c, animCtx), name)
, x18_nextAnim(b.lock())
, x20_animCtx(animCtx)
, x30_fundamentals(CSequenceHelper(x14_child, x18_nextAnim, animCtx).ComputeSequenceFundamentals()) {}

CAnimTreeLoopIn::CAnimTreeLoopIn(const std::weak_ptr<CAnimTreeNode>& a, const std::weak_ptr<CAnimTreeNode>& b,
                                 bool didLoopIn, CAnimSysContext animCtx, std::string_view name,
                                 CSequenceFundamentals fundamentals, const CCharAnimTime& time)
: CAnimTreeSingleChild(a, name)
, x18_nextAnim(b.lock())
, x1c_didLoopIn(didLoopIn)
, x20_animCtx(std::move(animCtx))
, x30_fundamentals(std::move(fundamentals))
, x88_curTime(time) {}

CAnimTreeEffectiveContribution CAnimTreeLoopIn::VGetContributionOfHighestInfluence() const {
  return x14_child->GetContributionOfHighestInfluence();
}

std::optional<std::unique_ptr<IAnimReader>> CAnimTreeLoopIn::VSimplified() {
  CCharAnimTime remTime = x14_child->VGetTimeRemaining();
  if (remTime.GreaterThanZero() && !remTime.EpsilonZero()) {
    auto simp = x14_child->Simplified();
    if (simp)
      x14_child = CAnimTreeNode::Cast(std::move(*simp));
  } else if (x1c_didLoopIn && x14_child->VGetTimeRemaining().EqualsZero()) {
    return x14_child->Clone();
  }
  return {};
}

std::shared_ptr<IAnimReader> CAnimTreeLoopIn::VGetBestUnblendedChild() const {
  if (std::shared_ptr<IAnimReader> bestChild = x14_child->GetBestUnblendedChild()) {
    return std::make_shared<CAnimTreeLoopIn>(CAnimTreeNode::Cast(bestChild->Clone()), x18_nextAnim, x1c_didLoopIn,
                                             x20_animCtx, x4_name, x30_fundamentals, x88_curTime);
  }
  return {};
}

std::unique_ptr<IAnimReader> CAnimTreeLoopIn::VClone() const {
  return std::make_unique<CAnimTreeLoopIn>(CAnimTreeNode::Cast(x14_child->Clone()), x18_nextAnim, x1c_didLoopIn,
                                           x20_animCtx, x4_name, x30_fundamentals, x88_curTime);
}

u32 CAnimTreeLoopIn::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator,
                                     u32 unk) const {
  return _getPOIList<CBoolPOINode>(time, listOut, capacity, iterator, unk, x30_fundamentals.GetBoolPointsOfInterest(),
                                   x88_curTime);
}

u32 CAnimTreeLoopIn::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator,
                                      u32 unk) const {
  return _getPOIList<CInt32POINode>(time, listOut, capacity, iterator, unk, x30_fundamentals.GetInt32PointsOfInterest(),
                                    x88_curTime);
}

u32 CAnimTreeLoopIn::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity,
                                         u32 iterator, u32 unk) const {
  return _getPOIList<CParticlePOINode>(time, listOut, capacity, iterator, unk,
                                       x30_fundamentals.GetParticlePointsOfInterest(), x88_curTime);
}

u32 CAnimTreeLoopIn::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator,
                                      u32 unk) const {
  return _getPOIList<CSoundPOINode>(time, listOut, capacity, iterator, unk, x30_fundamentals.GetSoundPointsOfInterest(),
                                    x88_curTime);
}

CSteadyStateAnimInfo CAnimTreeLoopIn::VGetSteadyStateAnimInfo() const {
  return x30_fundamentals.GetSteadyStateAnimInfo();
}

CCharAnimTime CAnimTreeLoopIn::VGetTimeRemaining() const {
  return x30_fundamentals.GetSteadyStateAnimInfo().GetDuration() - x88_curTime;
}

SAdvancementResults CAnimTreeLoopIn::VAdvanceView(const CCharAnimTime& dt) {
  std::shared_ptr<CAnimTreeNode> origChild = x14_child;
  SAdvancementResults res = origChild->VAdvanceView(dt);
  x88_curTime += dt - res.x0_remTime;
  CCharAnimTime remTime = origChild->VGetTimeRemaining();
  if ((remTime.EpsilonZero() || (dt - res.x0_remTime).EpsilonZero()) && !x1c_didLoopIn) {
    x14_child = CTreeUtils::GetTransitionTree(origChild, x18_nextAnim, x20_animCtx);
    x1c_didLoopIn = true;
  }
  return res;
}

} // namespace urde