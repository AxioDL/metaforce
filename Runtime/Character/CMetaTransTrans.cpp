#include "Runtime/Character/CMetaTransTrans.hpp"

#include "Runtime/Character/CAnimTreeTransition.hpp"

namespace metaforce {

CMetaTransTrans::CMetaTransTrans(CInputStream& in) {
  x4_transDur = in.Get<CCharAnimTime>();
  xc_ = in.ReadBool();
  xd_runA = in.ReadBool();
  x10_flags = in.ReadLong();
}

std::shared_ptr<CAnimTreeNode> CMetaTransTrans::VGetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                                                   const std::weak_ptr<CAnimTreeNode>& b,
                                                                   const CAnimSysContext& animSys) const {
  return std::make_shared<CAnimTreeTransition>(
      xc_, a, b, x4_transDur, xd_runA, x10_flags,
      CAnimTreeTransition::CreatePrimitiveName(a, b, x4_transDur.GetSeconds()));
}
} // namespace metaforce
