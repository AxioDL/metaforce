#include "CMetaTransPhaseTrans.hpp"
#include "CAnimTreeNode.hpp"

namespace urde
{

CMetaTransPhaseTrans::CMetaTransPhaseTrans(CInputStream& in)
{
    x4_ = in.readFloatBig();
    x8_ = in.readUint32Big();
    xc_ = in.readBool();
    xd_ = in.readBool();
    x10_ = in.readUint32Big();
}

std::shared_ptr<CAnimTreeNode>
CMetaTransPhaseTrans::VGetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                         const std::weak_ptr<CAnimTreeNode>& b,
                                         const CAnimSysContext& animSys) const
{
    return {};
}

}
