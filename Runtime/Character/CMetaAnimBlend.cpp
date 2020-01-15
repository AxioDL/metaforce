#include "Runtime/Character/CMetaAnimBlend.hpp"

#include "Runtime/Character/CAnimTreeBlend.hpp"
#include "Runtime/Character/CMetaAnimFactory.hpp"

namespace urde {

CMetaAnimBlend::CMetaAnimBlend(CInputStream& in) {
  x4_animA = CMetaAnimFactory::CreateMetaAnim(in);
  x8_animB = CMetaAnimFactory::CreateMetaAnim(in);
  xc_blend = in.readFloatBig();
  x10_ = in.readBool();
}

void CMetaAnimBlend::GetUniquePrimitives(std::set<CPrimitive>& primsOut) const {
  x4_animA->GetUniquePrimitives(primsOut);
  x4_animA->GetUniquePrimitives(primsOut);
}

std::shared_ptr<CAnimTreeNode> CMetaAnimBlend::VGetAnimationTree(const CAnimSysContext& animSys,
                                                                 const CMetaAnimTreeBuildOrders& orders) const {
  CMetaAnimTreeBuildOrders oa = CMetaAnimTreeBuildOrders::NoSpecialOrders();
  CMetaAnimTreeBuildOrders ob = orders.x0_recursiveAdvance ?
    CMetaAnimTreeBuildOrders::PreAdvanceForAll(*orders.x0_recursiveAdvance) :
    CMetaAnimTreeBuildOrders::NoSpecialOrders();
  auto a = x4_animA->GetAnimationTree(animSys, oa);
  auto b = x8_animB->GetAnimationTree(animSys, ob);
  return std::make_shared<CAnimTreeBlend>(x10_, a, b, xc_blend,
         CAnimTreeBlend::CreatePrimitiveName(a, b, xc_blend));
}

} // namespace urde
