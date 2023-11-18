#include "Runtime/Character/CMetaTransPhaseTrans.hpp"

#include "Runtime/Character/CAnimTreeNode.hpp"
#include "Runtime/Character/CAnimTreeTimeScale.hpp"
#include "Runtime/Character/CAnimTreeTransition.hpp"
#include "Runtime/Character/CTimeScaleFunctions.hpp"

namespace metaforce {

CMetaTransPhaseTrans::CMetaTransPhaseTrans(CInputStream& in) {
  x4_transDur = CCharAnimTime(in);
  xc_ = in.ReadBool();
  xd_runA = in.ReadBool();
  x10_flags = in.ReadLong();
}

std::shared_ptr<CAnimTreeNode> CMetaTransPhaseTrans::VGetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                                                        const std::weak_ptr<CAnimTreeNode>& b,
                                                                        const CAnimSysContext& animSys) const {
  std::shared_ptr<CAnimTreeNode> nA = a.lock();
  CAnimTreeEffectiveContribution cA = nA->GetContributionOfHighestInfluence();
  std::shared_ptr<CAnimTreeNode> nB = b.lock();
  CAnimTreeEffectiveContribution cB = nB->GetContributionOfHighestInfluence();
  float y2A = cA.GetSteadyStateAnimInfo().GetDuration() / cB.GetSteadyStateAnimInfo().GetDuration();
  float y1B = cB.GetSteadyStateAnimInfo().GetDuration() / cA.GetSteadyStateAnimInfo().GetDuration();

  nB->VSetPhase(zeus::clamp(0.f, 1.f - cA.GetTimeRemaining() / cA.GetSteadyStateAnimInfo().GetDuration(), 1.f));
  auto tsA = std::make_shared<CAnimTreeTimeScale>(
      a, std::make_unique<CLinearAnimationTimeScale>(CCharAnimTime{}, 1.f, x4_transDur, y2A), x4_transDur,
      CAnimTreeTimeScale::CreatePrimitiveName(a, 1.f, x4_transDur, y2A));
  auto tsB = std::make_shared<CAnimTreeTimeScale>(
      b, std::make_unique<CLinearAnimationTimeScale>(CCharAnimTime{}, y1B, x4_transDur, 1.f), x4_transDur,
      CAnimTreeTimeScale::CreatePrimitiveName(b, y1B, x4_transDur, 1.f));

  return std::make_shared<CAnimTreeTransition>(
      xc_, tsA, tsB, x4_transDur, xd_runA, x10_flags,
      CAnimTreeTransition::CreatePrimitiveName(tsA, tsB, x4_transDur.GetSeconds()));
}

} // namespace metaforce
