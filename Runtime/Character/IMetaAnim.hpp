#ifndef __PSHAG_IMETAANIM_HPP__
#define __PSHAG_IMETAANIM_HPP__

#include "../RetroTypes.hpp"
#include <set>

namespace urde
{
class CAnimTreeNode;
class CAnimSysContext;
class CMetaAnimTreeBuildOrders;
class CPrimitive;

enum class EMetaAnimType
{
    Primitive,
    Blend,
    PhaseBlend,
    Random,
    Sequence
};

class IMetaAnim
{
public:
    virtual ~IMetaAnim() = default;
    virtual std::shared_ptr<CAnimTreeNode> GetAnimationTree(const CAnimSysContext& animSys,
                                                            const CMetaAnimTreeBuildOrders& orders) const=0;
    virtual void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const=0;
    virtual EMetaAnimType GetType() const=0;
    virtual std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                             const CMetaAnimTreeBuildOrders& orders) const=0;
};

}

#endif // __PSHAG_IMETAANIM_HPP__
