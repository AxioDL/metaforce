#include "Runtime/Character/CMetaAnimPhaseBlend.hpp"

#include "Runtime/Character/CAnimTreeBlend.hpp"
#include "Runtime/Character/CAnimTreeTimeScale.hpp"
#include "Runtime/Character/CMetaAnimFactory.hpp"

namespace urde {

CMetaAnimPhaseBlend::CMetaAnimPhaseBlend(CInputStream& in) {
  x4_animA = CMetaAnimFactory::CreateMetaAnim(in);
  x8_animB = CMetaAnimFactory::CreateMetaAnim(in);
  xc_blend = in.readFloatBig();
  x10_ = in.readBool();
}

void CMetaAnimPhaseBlend::GetUniquePrimitives(std::set<CPrimitive>& primsOut) const {
  x4_animA->GetUniquePrimitives(primsOut);
  x8_animB->GetUniquePrimitives(primsOut);
}

std::shared_ptr<CAnimTreeNode> CMetaAnimPhaseBlend::VGetAnimationTree(const CAnimSysContext& animSys,
                                                                      const CMetaAnimTreeBuildOrders& orders) const {
  if (orders.x0_recursiveAdvance)
    return GetAnimationTree(animSys, CMetaAnimTreeBuildOrders::PreAdvanceForAll(*orders.x0_recursiveAdvance));

  auto a = x4_animA->GetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders());
  auto b = x8_animB->GetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders());
  auto da = a->GetContributionOfHighestInfluence().GetSteadyStateAnimInfo().GetDuration();
  auto db = b->GetContributionOfHighestInfluence().GetSteadyStateAnimInfo().GetDuration();
  auto dblend = da + (db - da) * xc_blend;
  float fa = da / dblend;
  float fb = db / dblend;

  auto tsa = std::make_shared<CAnimTreeTimeScale>(a, fa,
    CAnimTreeTimeScale::CreatePrimitiveName(a, fa, CCharAnimTime::Infinity(), -1.f));
  auto tsb = std::make_shared<CAnimTreeTimeScale>(b, fb,
    CAnimTreeTimeScale::CreatePrimitiveName(b, fb, CCharAnimTime::Infinity(), -1.f));

  return std::make_shared<CAnimTreeBlend>(x10_, tsa, tsb, xc_blend,
                                          CAnimTreeBlend::CreatePrimitiveName(tsa, tsb, xc_blend));
}

} // namespace urde
