#ifndef __PSHAG_IMETATRANS_HPP__
#define __PSHAG_IMETATRANS_HPP__

#include "../RetroTypes.hpp"

namespace urde
{
class CAnimTreeNode;
class CAnimSysContext;

enum class EMetaTransType
{
    MetaAnim,
    Trans,
    PhaseTrans,
    Snap
};

class IMetaTrans
{
public:
    virtual ~IMetaTrans() = default;
    virtual std::shared_ptr<CAnimTreeNode> VGetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                                              const std::weak_ptr<CAnimTreeNode>& b,
                                                              const CAnimSysContext& animSys) const=0;
    virtual EMetaTransType GetType() const=0;
};

}

#endif // __PSHAG_IMETATRANS_HPP__
