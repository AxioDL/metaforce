#include "CMetaTransTrans.hpp"
#include "CAnimTreeTransition.hpp"

namespace urde
{

CMetaTransTrans::CMetaTransTrans(CInputStream& in)
{
    x4_animTime = CCharAnimTime(in);
    xc_ = in.readBool();
    xd_ = in.readBool();
    x10_ = in.readUint32Big();
}

std::shared_ptr<CAnimTreeNode> CMetaTransTrans::VGetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                                                   const std::weak_ptr<CAnimTreeNode>& b,
                                                                   const CAnimSysContext& animSys) const
{
    return std::make_shared<CAnimTreeTransition>(xc_, a, b, x4_animTime, xd_, x10_,
                                           CAnimTreeTransition::CreatePrimitiveName(a, b, x4_animTime));
}
}
