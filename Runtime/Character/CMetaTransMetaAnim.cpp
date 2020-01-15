#include "Runtime/Character/CMetaTransMetaAnim.hpp"

#include "Runtime/Character/CAnimTreeLoopIn.hpp"
#include "Runtime/Character/CMetaAnimFactory.hpp"

namespace urde {

CMetaTransMetaAnim::CMetaTransMetaAnim(CInputStream& in) : x4_metaAnim(CMetaAnimFactory::CreateMetaAnim(in)) {}

std::shared_ptr<CAnimTreeNode> CMetaTransMetaAnim::VGetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                                                      const std::weak_ptr<CAnimTreeNode>& b,
                                                                      const CAnimSysContext& animSys) const {
  std::shared_ptr<CAnimTreeNode> animNode =
      x4_metaAnim->GetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders());
  return std::make_shared<CAnimTreeLoopIn>(a, b, animNode, animSys,
                                           CAnimTreeLoopIn::CreatePrimitiveName(a, b, animNode));
}

} // namespace urde
