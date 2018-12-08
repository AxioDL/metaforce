#include "CAnimTreeSequence.hpp"
#include "IMetaAnim.hpp"
#include "CAnimSysContext.hpp"
#include "CTreeUtils.hpp"

namespace urde {

CAnimTreeSequence::CAnimTreeSequence(const std::vector<std::shared_ptr<IMetaAnim>>& seq, const CAnimSysContext& animSys,
                                     std::string_view name)
: CAnimTreeSingleChild(seq[0]->GetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders()), name)
, x18_animCtx(animSys)
, x28_sequence(seq)
, x3c_fundamentals(CSequenceHelper(seq, animSys).ComputeSequenceFundamentals())
, x94_curTime(0.f) {}

CAnimTreeSequence::CAnimTreeSequence(const std::shared_ptr<CAnimTreeNode>& curNode,
                                     const std::vector<std::shared_ptr<IMetaAnim>>& metaAnims,
                                     const CAnimSysContext& animSys, std::string_view name,
                                     const CSequenceFundamentals& fundamentals, const CCharAnimTime& time)
: CAnimTreeSingleChild(curNode, name)
, x18_animCtx(animSys)
, x28_sequence(metaAnims)
, x3c_fundamentals(fundamentals)
, x94_curTime(time) {}

CAnimTreeEffectiveContribution CAnimTreeSequence::VGetContributionOfHighestInfluence() const {
  return x14_child->GetContributionOfHighestInfluence();
}

std::shared_ptr<IAnimReader> CAnimTreeSequence::VGetBestUnblendedChild() const {
  std::shared_ptr<IAnimReader> ch = x14_child->GetBestUnblendedChild();
  if (!ch)
    return ch;
  return std::make_shared<CAnimTreeSequence>(
      std::static_pointer_cast<CAnimTreeNode>(std::shared_ptr<IAnimReader>(ch->Clone())), x28_sequence, x18_animCtx,
      x4_name, x3c_fundamentals, x94_curTime);
}

SAdvancementResults CAnimTreeSequence::VAdvanceView(const CCharAnimTime& dt) {
  CCharAnimTime totalDelta;
  zeus::CVector3f posDelta;
  zeus::CQuaternion rotDelta;

  std::shared_ptr<CAnimTreeNode> curChild = x14_child;
  if (x38_curIdx >= x28_sequence.size() && curChild->VGetTimeRemaining().EqualsZero()) {
    x3c_fundamentals = CSequenceHelper(x28_sequence, x18_animCtx).ComputeSequenceFundamentals();
    x38_curIdx = 0;
    x14_child = CTreeUtils::GetTransitionTree(
        curChild, x28_sequence[x38_curIdx]->GetAnimationTree(x18_animCtx, CMetaAnimTreeBuildOrders::NoSpecialOrders()),
        x18_animCtx);
    curChild = x14_child;
  }

  CCharAnimTime remTime = dt;
  while (remTime.GreaterThanZero() && x38_curIdx < x28_sequence.size()) {
    CCharAnimTime chRem = curChild->VGetTimeRemaining();
    if (chRem.EqualsZero()) {
      ++x38_curIdx;
      if (x38_curIdx < x28_sequence.size()) {
        x14_child = CTreeUtils::GetTransitionTree(
            curChild,
            x28_sequence[x38_curIdx]->GetAnimationTree(x18_animCtx, CMetaAnimTreeBuildOrders::NoSpecialOrders()),
            x18_animCtx);
      }
    }
    curChild = x14_child;
    if (x38_curIdx < x28_sequence.size()) {
      SAdvancementResults res = curChild->VAdvanceView(remTime);
      if (auto simp = curChild->Simplified()) {
        curChild = CAnimTreeNode::Cast(std::move(*simp));
        x14_child = curChild;
      }
      CCharAnimTime prevRemTime = remTime;
      remTime = res.x0_remTime;
      totalDelta += prevRemTime - remTime;
      posDelta += res.x8_deltas.x0_posDelta;
      rotDelta = rotDelta * res.x8_deltas.xc_rotDelta;
    }
  }

  x94_curTime += totalDelta;
  return {dt - totalDelta, {posDelta, rotDelta}};
}

CCharAnimTime CAnimTreeSequence::VGetTimeRemaining() const {
  if (x38_curIdx == x28_sequence.size() - 1)
    return x14_child->VGetTimeRemaining();
  return x3c_fundamentals.GetSteadyStateAnimInfo().GetDuration() - x94_curTime.GetSeconds();
}

CSteadyStateAnimInfo CAnimTreeSequence::VGetSteadyStateAnimInfo() const {
  return x3c_fundamentals.GetSteadyStateAnimInfo();
}

u32 CAnimTreeSequence::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator,
                                       u32 unk) const {
  return _getPOIList(time, listOut, capacity, iterator, unk, x3c_fundamentals.GetBoolPointsOfInterest(), x94_curTime);
}

u32 CAnimTreeSequence::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator,
                                        u32 unk) const {
  return _getPOIList(time, listOut, capacity, iterator, unk, x3c_fundamentals.GetInt32PointsOfInterest(), x94_curTime);
}

u32 CAnimTreeSequence::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity,
                                           u32 iterator, u32 unk) const {
  return _getPOIList(time, listOut, capacity, iterator, unk, x3c_fundamentals.GetParticlePointsOfInterest(),
                     x94_curTime);
}

u32 CAnimTreeSequence::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator,
                                        u32 unk) const {
  return _getPOIList(time, listOut, capacity, iterator, unk, x3c_fundamentals.GetSoundPointsOfInterest(), x94_curTime);
}

std::unique_ptr<IAnimReader> CAnimTreeSequence::VClone() const {
  return std::make_unique<CAnimTreeSequence>(
      std::static_pointer_cast<CAnimTreeNode>(std::shared_ptr<IAnimReader>(x14_child->Clone())), x28_sequence,
      x18_animCtx, x4_name, x3c_fundamentals, x94_curTime);
}

} // namespace urde
