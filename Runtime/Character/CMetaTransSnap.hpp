#ifndef __URDE_CMETATRANSSNAP_HPP__
#define __URDE_CMETATRANSSNAP_HPP__

#include "IMetaTrans.hpp"
#include "IOStreams.hpp"

namespace urde
{

class CMetaTransSnap : public IMetaTrans
{
public:
    EMetaTransType GetType() const {return EMetaTransType::Snap;}

    std::shared_ptr<CAnimTreeNode> VGetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                                      const std::weak_ptr<CAnimTreeNode>& b,
                                                      const CAnimSysContext& animSys) const;
};

}

#endif // __URDE_CMETATRANSSNAP_HPP__
