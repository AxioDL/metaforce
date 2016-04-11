#include "CMetaTransSnap.hpp"

namespace urde
{

std::shared_ptr<CAnimTreeNode>
CMetaTransSnap::VGetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                   const std::weak_ptr<CAnimTreeNode>& b,
                                   const CAnimSysContext& animSys) const
{
    return b.lock();
}

}
